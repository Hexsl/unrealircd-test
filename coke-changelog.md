# coca-cola.colored.piss.hexick.com Custom UnrealIRCd Changelog

## Finished variations relative to original repo:

* **servicebot.c** - has been altered to allow O-lined users to toggle the `S` ("$client is a Network Service" + protection) mode on themselves.
* **api-usermodes.c** - has been altered to allow opers to toggle the `r` (registered user) mode.
* **conf.c** - has been changed to allow the above modes to be included within the `modes` parameter of oper blocks.

## WIP changes:

* Allow opers to execute most svs* commands through modules (see above); ensure appropriate limits and syncing to prevent `/OS RAW`-esque damage.
* Create logs for new user commands.
* Start desting and debugging the new qline.c module.
