#!/usr/bin/env bash
# Source this file before launching the AMR:
#   source /var/amr/config/amr_config.sh

export AMR_ID="amr_01"
# Model key from ${CONFIG_FOLDER}/amr_lidars.yaml:
# ydlidar_x4, sick_tim571, a2m8, a1, s2
export LIDAR_TYPE="a2m8"

# Shared directory for all AMR runtime configuration files.
export CONFIG_FOLDER="/var/amr/config"
