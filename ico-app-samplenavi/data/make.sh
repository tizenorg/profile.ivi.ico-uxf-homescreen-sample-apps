#!/bin/sh
edje_cc -id ../res/images/ -fd ../res/images/ samplenavi_conf.edc
edje_cc -id ../res/images/ -fd ../res/images/ samplenavi_conf_dist.edc
chmod 755 samplenavi_conf.edj
chmod 755 samplenavi_conf_dist.edj
