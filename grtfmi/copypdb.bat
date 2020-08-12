rem Utility script to copy files with forward slashes and ignore
rem files that don't exist

set a=%1
set b=%2

set a=%a:/=\%
set b=%b:/=\%

if exist %a% (
  copy %a% %b%
)
