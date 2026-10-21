"""Check driver selection without installing vendor drivers or accessing hardware."""

from pathlib import Path
import runpy

from ament_index_python.packages import PackageNotFoundError
from launch import LaunchContext
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.utilities import perform_substitutions
import pytest
import yaml


PACKAGE_ROOT = Path(__file__).resolve().parents[1]


@pytest.fixture
def selector():
    return runpy.run_path(str(PACKAGE_ROOT / 'launch' / 'lidar.launch.py'))


def context_for(model, path):
    context = LaunchContext()
    context.launch_configurations.update(lidar_type=model, config_file=str(path))
    return context


@pytest.mark.parametrize('model', ['ydlidar_x4', 'sick_tim571', 'a2m8', 'a1', 's2'])
def test_includes_only_selected_driver(selector, monkeypatch, tmp_path, model):
    registry = PACKAGE_ROOT / 'config' / 'config.yaml'
    driver = yaml.safe_load(registry.read_text())['drivers'][model]
    launch_file = tmp_path / driver['launch_file']
    launch_file.parent.mkdir(parents=True)
    launch_file.write_text(
        'from launch import LaunchDescription\n'
        'def generate_launch_description():\n'
        '    return LaunchDescription([])\n')
    lookups = []

    def get_share(package):
        lookups.append(package)
        assert package == driver['package']
        return str(tmp_path)

    callback = selector['launch_selected_lidar']
    monkeypatch.setitem(callback.__globals__, 'get_package_share_directory', get_share)
    monkeypatch.setenv('CONFIG_FOLDER', str(tmp_path / 'robot_config'))
    context = context_for(f' {model.upper()} ', registry)
    actions = callback(context)
    includes = [action for action in actions if isinstance(action, IncludeLaunchDescription)]
    assert len(includes) == 1
    include = includes[0]
    # Load the fake driver's launch to verify that the selected include is usable.
    assert not include.launch_description_source.get_launch_description(context).entities
    assert include.launch_description_source.location == str(launch_file)
    assert lookups == [driver['package']]
    if model == 'ydlidar_x4':
        assert dict(include.launch_arguments)['params_file'] == str(
            tmp_path / 'robot_config' / 'ydlidar_x4.yaml')


def test_environment_defaults_do_not_require_driver(selector, monkeypatch):
    monkeypatch.setenv('CONFIG_FOLDER', '/tmp/lidar_config')
    monkeypatch.setenv('LIDAR_TYPE', 'sick_tim571')
    description = selector['generate_launch_description']()
    arguments = {
        action.name: perform_substitutions(LaunchContext(), action.default_value)
        for action in description.entities if isinstance(action, DeclareLaunchArgument)
    }
    assert arguments['lidar_type'] == 'sick_tim571'
    assert arguments['config_file'] == '/tmp/lidar_config/amr_lidars.yaml'


def test_missing_driver_has_actionable_error(selector, monkeypatch):
    callback = selector['launch_selected_lidar']

    def missing(package):
        raise PackageNotFoundError(package)

    monkeypatch.setitem(callback.__globals__, 'get_package_share_directory', missing)
    context = context_for('sick_tim571', PACKAGE_ROOT / 'config' / 'config.yaml')
    with pytest.raises(RuntimeError, match='Add the driver source'):
        callback(context)


def test_missing_launch_file_has_actionable_error(selector, monkeypatch, tmp_path):
    callback = selector['launch_selected_lidar']
    monkeypatch.setitem(
        callback.__globals__, 'get_package_share_directory', lambda package: str(tmp_path))
    context = context_for('ydlidar_x4', PACKAGE_ROOT / 'config' / 'config.yaml')
    with pytest.raises(RuntimeError, match='Launch file.*does not exist'):
        callback(context)


@pytest.mark.parametrize('content, message', [
    ('[]', 'drivers'),
    ('drivers: {}', 'drivers'),
    ('drivers: {other: {}}', 'Unsupported LIDAR_TYPE'),
    ('drivers: {ydlidar_x4: null}', 'must be a mapping'),
    ('drivers: {ydlidar_x4: {}}', 'package'),
    ('drivers: {ydlidar_x4: {package: driver}}', 'launch_file'),
    ('drivers: {ydlidar_x4: {package: driver, launch_file: a.py, launch_arguments: []}}',
     'launch_arguments'),
    ('drivers: [', 'Cannot read configuration'),
])
def test_rejects_invalid_registry(selector, tmp_path, content, message):
    registry = tmp_path / 'amr_lidars.yaml'
    registry.write_text(content)
    with pytest.raises(RuntimeError, match=message):
        selector['launch_selected_lidar'](context_for('ydlidar_x4', registry))


def test_missing_registry_has_actionable_error(selector, tmp_path):
    with pytest.raises(RuntimeError, match='Cannot read configuration'):
        selector['launch_selected_lidar'](context_for('ydlidar_x4', tmp_path / 'missing.yaml'))
