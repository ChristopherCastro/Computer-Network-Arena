## Server Test
S WAIT
S URTURN 20 5 5 10 1 			; pattern [S URTURN life x y ammo opponents]	
S URTURN 20 6 7 10 1
S SEEK RETURNED Enemigo 2
S SHOOT HIT Enemigo 2
S URTURN 20 7 7 10 1
S URTURN 20 7 6 10 1
S URTURN 20 7 5 10 1

## Compile Robot
$ make robot=YourRobotName

(Make sure `./robots/YourRobotName.c` exists)

#### Example:
$ make robot=Waller