# coca-cola.colored.piss.hexick.com Custom UnrealIRCd Changelog

## Finished variations relative to original repo:

* **servicebot.c** - has been altered to allow O-lined users to toggle the `S` ("$client is a Network Service" + protection) mode on themselves.
* **api-usermodes.c** - has been altered to allow opers to toggle the `r` (registered user) mode.

## WIP changes:

* Augment **swhois.c** to change `/swhois` from a server command to a user-executiable one by opers-only.
* Allow opers to execute most svs* commands (see above); ensure appropriate limits and syncing to prevent `/OS RAW`-esque damage.
* Create logs for new user commands.
