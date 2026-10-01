This file describes changes in the meataxe64 package.

## Unreleased

- Require GAP >= 4.12; fix compatibility with GAP 4.12 and 4.13 (#44)
- Fix crashes and format errors in error messages (#48)
- Fix a garbage collection bug when creating meataxe64 objects
- Add floating-point based matrix multiplication kernel, used for
  characteristic 67 and above on suitable CPUs, with a workaround for a
  compiler bug that caused segfaults in it
- Add `Characteristic` methods for meataxe64 fields, field elements and
  matrices
- `MTX64_WriteMatrix` and `MTX64_ReadMatrix` now check the file path and raise
  an error instead of exiting GAP (#6)
- Swap the arguments of `Randomize` so the random source comes first, keeping
  the old order for backwards compatibility
- Drop the AutoDoc dependency for loading the package

## 0.1 (2019-08-23)
