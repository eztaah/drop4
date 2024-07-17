# drop4

This game is developed in C and compiled to WebAssembly to be playable on the web.

The game is available at this URL: https://eztaah.github.io/drop4/en/

<br>

## How to build from source : 

*(Instructions intended for Linux users.)*

Navigate to the project directory and execute these commands :

```shell
$ make release
$ cp src/assets/game_icon.png out/web/en/
$ python -m http.server 8080
``` 

Once this is done, open a web browser and enter this URL :
http://localhost:8080/out/web/en/game.html
