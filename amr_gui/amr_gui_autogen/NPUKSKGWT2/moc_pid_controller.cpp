/****************************************************************************
** Meta object code from reading C++ file 'pid_controller.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.2.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../controller/pid_controller.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'pid_controller.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.2.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_PidController_t {
    const uint offsetsAndSize[48];
    char stringdata0[317];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(offsetof(qt_meta_stringdata_PidController_t, stringdata0) + ofs), len 
static const qt_meta_stringdata_PidController_t qt_meta_stringdata_PidController = {
    {
QT_MOC_LITERAL(0, 13), // "PidController"
QT_MOC_LITERAL(14, 21), // "wheelTelemetryChanged"
QT_MOC_LITERAL(36, 0), // ""
QT_MOC_LITERAL(37, 24), // "setWheelTelemetryChanged"
QT_MOC_LITERAL(62, 20), // "isInitializedChanged"
QT_MOC_LITERAL(83, 18), // "initialGainsLoaded"
QT_MOC_LITERAL(102, 20), // "updateWheelTelemetry"
QT_MOC_LITERAL(123, 31), // "amr_common::msg::WheelTelemetry"
QT_MOC_LITERAL(155, 3), // "msg"
QT_MOC_LITERAL(159, 11), // "setPidGains"
QT_MOC_LITERAL(171, 10), // "wheelIndex"
QT_MOC_LITERAL(182, 2), // "kp"
QT_MOC_LITERAL(185, 2), // "ki"
QT_MOC_LITERAL(188, 2), // "kd"
QT_MOC_LITERAL(191, 16), // "publishSetPoints"
QT_MOC_LITERAL(208, 13), // "syncSetPoints"
QT_MOC_LITERAL(222, 7), // "wheelFL"
QT_MOC_LITERAL(230, 15), // "WheelDataModel*"
QT_MOC_LITERAL(246, 7), // "wheelFR"
QT_MOC_LITERAL(254, 7), // "wheelRL"
QT_MOC_LITERAL(262, 7), // "wheelRR"
QT_MOC_LITERAL(270, 14), // "wheelTelemetry"
QT_MOC_LITERAL(285, 17), // "setWheelTelemetry"
QT_MOC_LITERAL(303, 13) // "isInitialized"

    },
    "PidController\0wheelTelemetryChanged\0"
    "\0setWheelTelemetryChanged\0"
    "isInitializedChanged\0initialGainsLoaded\0"
    "updateWheelTelemetry\0"
    "amr_common::msg::WheelTelemetry\0msg\0"
    "setPidGains\0wheelIndex\0kp\0ki\0kd\0"
    "publishSetPoints\0syncSetPoints\0wheelFL\0"
    "WheelDataModel*\0wheelFR\0wheelRL\0wheelRR\0"
    "wheelTelemetry\0setWheelTelemetry\0"
    "isInitialized"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PidController[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       7,   80, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   62,    2, 0x06,    8 /* Public */,
       3,    0,   63,    2, 0x06,    9 /* Public */,
       4,    0,   64,    2, 0x06,   10 /* Public */,
       5,    0,   65,    2, 0x06,   11 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       6,    1,   66,    2, 0x0a,   12 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       9,    4,   69,    2, 0x02,   14 /* Public */,
      14,    0,   78,    2, 0x02,   19 /* Public */,
      15,    0,   79,    2, 0x02,   20 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 7,    8,

 // methods: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Double, QMetaType::Double, QMetaType::Double,   10,   11,   12,   13,
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags
      16, 0x80000000 | 17, 0x00015409, uint(-1), 0,
      18, 0x80000000 | 17, 0x00015409, uint(-1), 0,
      19, 0x80000000 | 17, 0x00015409, uint(-1), 0,
      20, 0x80000000 | 17, 0x00015409, uint(-1), 0,
      21, QMetaType::QVariantMap, 0x00015001, uint(0), 0,
      22, QMetaType::QVariantMap, 0x00015103, uint(1), 0,
      23, QMetaType::Bool, 0x00015001, uint(2), 0,

       0        // eod
};

void PidController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PidController *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->wheelTelemetryChanged(); break;
        case 1: _t->setWheelTelemetryChanged(); break;
        case 2: _t->isInitializedChanged(); break;
        case 3: _t->initialGainsLoaded(); break;
        case 4: _t->updateWheelTelemetry((*reinterpret_cast< std::add_pointer_t<amr_common::msg::WheelTelemetry>>(_a[1]))); break;
        case 5: _t->setPidGains((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[4]))); break;
        case 6: _t->publishSetPoints(); break;
        case 7: _t->syncSetPoints(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PidController::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PidController::wheelTelemetryChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (PidController::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PidController::setWheelTelemetryChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (PidController::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PidController::isInitializedChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (PidController::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PidController::initialGainsLoaded)) {
                *result = 3;
                return;
            }
        }
    } else if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 3:
        case 2:
        case 1:
        case 0:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< WheelDataModel* >(); break;
        }
    }

#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<PidController *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< WheelDataModel**>(_v) = _t->wheelFL(); break;
        case 1: *reinterpret_cast< WheelDataModel**>(_v) = _t->wheelFR(); break;
        case 2: *reinterpret_cast< WheelDataModel**>(_v) = _t->wheelRL(); break;
        case 3: *reinterpret_cast< WheelDataModel**>(_v) = _t->wheelRR(); break;
        case 4: *reinterpret_cast< QVariantMap*>(_v) = _t->wheelTelemetry(); break;
        case 5: *reinterpret_cast< QVariantMap*>(_v) = _t->setWheelTelemetry(); break;
        case 6: *reinterpret_cast< bool*>(_v) = _t->isInitialized(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<PidController *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 5: _t->setSetWheelTelemetry(*reinterpret_cast< QVariantMap*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
#endif // QT_NO_PROPERTIES
}

const QMetaObject PidController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_PidController.offsetsAndSize,
    qt_meta_data_PidController,
    qt_static_metacall,
    nullptr,
qt_incomplete_metaTypeArray<qt_meta_stringdata_PidController_t
, QtPrivate::TypeAndForceComplete<WheelDataModel*, std::true_type>, QtPrivate::TypeAndForceComplete<WheelDataModel*, std::true_type>, QtPrivate::TypeAndForceComplete<WheelDataModel*, std::true_type>, QtPrivate::TypeAndForceComplete<WheelDataModel*, std::true_type>, QtPrivate::TypeAndForceComplete<QVariantMap, std::true_type>, QtPrivate::TypeAndForceComplete<QVariantMap, std::true_type>, QtPrivate::TypeAndForceComplete<bool, std::true_type>, QtPrivate::TypeAndForceComplete<PidController, std::true_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>
, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<const amr_common::msg::WheelTelemetry &, std::false_type>
, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<int, std::false_type>, QtPrivate::TypeAndForceComplete<double, std::false_type>, QtPrivate::TypeAndForceComplete<double, std::false_type>, QtPrivate::TypeAndForceComplete<double, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>, QtPrivate::TypeAndForceComplete<void, std::false_type>

>,
    nullptr
} };


const QMetaObject *PidController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PidController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PidController.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int PidController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void PidController::wheelTelemetryChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void PidController::setWheelTelemetryChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void PidController::isInitializedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void PidController::initialGainsLoaded()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
