# Third-party runtime

`vendor/js-dos/` contains the unmodified browser runtime files from **js-dos 6.22.60**, downloaded from the npm package `js-dos`:

- Package: https://registry.npmjs.org/js-dos/-/js-dos-6.22.60.tgz
- Project and source: https://github.com/caiiiycuk/js-dos
- DOSBox backend source: https://github.com/js-dos/dosbox
- License: GNU General Public License, version 2. See `vendor/js-dos/LICENSE`.

## Corresponding source

[`vendor/js-dos/source-6.22.60.tar.gz`](vendor/js-dos/source-6.22.60.tar.gz) provides the matching upstream source code, bundled dependencies, license notices and build scripts alongside the runtime. It was packaged from the upstream **6.22.60** tag (`d9d1045ef321bcde3f061dc8e686ded890147d6a`):

https://github.com/caiiiycuk/js-dos/tree/6.22.60

The archive includes the DOSBox C/C++ backend, TypeScript API, CMake and gulp build files, and the upstream build instructions in its `README.md`. Those instructions specify Emscripten 1.39.4 and Node 11.8.0. A rebuild with current tools has not been attempted.

Only unrelated sample game ZIPs, test PNGs and precompiled benchmark/DOS-extender executables were excluded from this source archive. Source code and build scripts are unchanged. The three browser runtime files in this repository are unmodified copies from the npm package.

Source archive SHA-256: `9943d19c756292620f099c963c9b0fdc6a414e7c6bb2f6ccf94f88139279419b`.

## Original DOS program

The host page is separate from the emulator. The original DOS executable incorporates the graphics and font routines provided by its original Borland toolchain; the original Borland library source is not included in this archive.
