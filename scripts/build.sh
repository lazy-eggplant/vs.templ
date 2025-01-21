#!/bin/bash
meson setup -Db_lto=true -Db_thinlto_cache=true -Db_lto_mode=thin --reconfigure build/ --buildtype=release
sudo meson install -C build