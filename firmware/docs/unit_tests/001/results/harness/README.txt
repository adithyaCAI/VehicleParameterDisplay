The Ceedling project these tests ran under.

project.yml   the configuration this run generated (paths are relative to
              the run directory: put the extracted firmware archive at
              repo/ and the test files at test/ beside it)
support/      host stand-ins for the vendor SDK headers the firmware
              includes, synthesized for this run

Re-run with:  ceedling test:all
