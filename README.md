# Generator for EV station map files

This utility generates a single JavaScript file allowing filtered download
of GPX- and KML- map files for EV charger stations listed by [nobil.no](https://info.nobil.no) .

## How to build

<pre>
qmake
make all install
</pre>

## Usage

<pre>
nobild -o $PWD/ev_charger_stations.js -a <APIKEY>
</pre>

## Supported platforms
<ul>
<li>FreeBSD</li>
</ul>

## Software support
Please contact hps@selasky.org.

--HPS
