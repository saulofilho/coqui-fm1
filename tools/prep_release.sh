#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 COQUÍ
# COQUÍ: everything that shows the version, made again for release X.Y before its tag (the tag's own build is GitHub's:
# .github/workflows/release.yml). After it: look at the diff, commit, then git tag vX.Y && git push origin vX.Y.
#   tools/prep_release.sh 1.0
#   - tools/build.py's DEFAULT_VERSION (local builds say X.Y)
#   - the host tests (tests/run_host.sh, APP_VER=X.Y)
#   - docs/card.png (the hub's card: the boot screen, the coquí singing) and docs/screens/boot.png
#   - the web emulator (web/build_web.sh, APP_VER=X.Y) and the hub (tools/gen_hub.py --version X.Y: its data, sounds, emu/)
# Needs what the build, the tests, the web build and gen_hub need (docs/BUILDING.md, README.md: The hub page).
set -e
cd "$(dirname "$0")/.."
V="$1"
echo "$V" | grep -Eq '^[0-9]\.[0-9](\.[0-9])?$' || { echo "prep_release: give X.Y (or X.Y.Z), e.g. 1.0"; exit 2; }
PY="${PYTHON:-python3}"
sed -i "s/^DEFAULT_VERSION = \"[^\"]*\"/DEFAULT_VERSION = \"$V\"/" tools/build.py
./build.sh --release "$V"
APP_VER="$V" sh tests/run_host.sh > build/host_test.log 2>&1 || { tail -20 build/host_test.log; echo "prep_release: tests FAILED"; exit 1; }
grep -E 'test: (passed|FAILED)' build/host_test.log
mkdir -p build/release
printf '2400 end\n' > build/release/boot.txt     # (2.3 s into the boot, frame 23: the coquí's QUÍ!)
build/host/coqui build/release/boot build/release/boot.txt 10 > /dev/null
"$PY" - <<'PY'
from PIL import Image
im = Image.open("build/release/boot-00023.ppm").convert("RGB")
im.save("docs/card.png", optimize=True)
im.resize((480, 480), Image.NEAREST).save("docs/screens/boot.png", optimize=True)
print("card -> docs/card.png, docs/screens/boot.png")
PY
APP_VER="$V" sh web/build_web.sh
"$PY" tools/gen_hub.py --version "$V"
echo "prep_release: $V ready; git diff, commit, then: git tag v$V && git push origin v$V"
