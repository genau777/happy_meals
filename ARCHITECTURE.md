# HappyMeals: client-server split

The project is split into two independent applications:

- `HappyMealsServer` is a console TCP server. It owns the database, recipe filtering, authentication, statistics and command parsing.
- `HappyMealsClient` is a Qt Widgets GUI client. It owns only the interface and local UI state, and talks to the server through `ClientApi` over TCP.

## Build

Build both applications:

```bash
qmake HappyMeals.pro
make
```

Build only the server:

```bash
qmake HappyMealsServer.pro
make
```

Build only the client:

```bash
qmake HappyMealsClient.pro
make
```

## Run On Different Devices

Start the server on the machine that stores the database:

```bash
./HappyMealsServer
```

The server listens on all interfaces on port `33333`.

Start the client on the same machine:

```bash
./HappyMealsClient
```

Start the client on another machine by passing the server IP:

```bash
./HappyMealsClient 192.168.1.10 33333
```

Make sure the server machine firewall allows incoming TCP connections to port `33333`.

## Current Protocol

Commands are sent as UTF-8 text lines:

- `auth:login,password`
- `reg:login,password,email`
- `get_dish:excludedIngredients;cuisines;maxTime`
- `dish_details:dishName`
- `get_stat`
- `add_favorite:dishName`
- `remove_favorite:dishName`
- `get_favorites`

Responses start with `OK:` or `ERROR:`.
