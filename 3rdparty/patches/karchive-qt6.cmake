# Qt 6.12 requires an explicit integer conversion when formatting QFlags.
# Keep this patch idempotent so dependency updates can safely rerun it.
foreach(name karchive kar krcc)
  set(path "${SOURCE_DIR}/karchive/src/${name}.cpp")
  file(READ "${path}" contents)
  string(REPLACE ".arg(mode)" ".arg(static_cast<int>(mode))" contents "${contents}")
  string(REPLACE ".arg(d->mode)" ".arg(static_cast<int>(d->mode))" contents "${contents}")
  file(WRITE "${path}" "${contents}")
endforeach()
