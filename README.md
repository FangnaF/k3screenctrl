# k3screenctrl
LCD screen controller for PHICOMM K3

This is a modified version of [updating](https://github.com/updateing/k3screenctrl)'s work, with a few fix and some new functions:
1. Full support for six pages: basic, port, wan, wifi, weather, hosts
2. *[New]* Switch to user-defined home page on middle key press
3. *[New]* Calling scripts on long press of left and right key

# Usage
```
USAGE: k3screenctrl [OPTIONS]\n\n
        \t-h, --help\t\t\tShow this help\n
        \t-r, --skip-reset\t\tDo not reset screen on startup (will reset 
        by default)\n
        \t-f, --foreground\t\tRun in foreground and print logs to stderr 
        as well\n
        \t-t, --test\t\t\tTest the scripts: collect info and print them, then 
        exit\n
        \t-d, --update-interval <SECS>\tCall data collection scripts 
        corresponding to current page and update content every SECS seconds\n
        \t-m, --screen-timeout <SECS>\tTurn off screen after this period of 
        time if there isn't any user interaction\n
        \t-o, --home-page <NUM>\t\tDefine as home page, the value is from %d to %d
        This page is switched to when middle button is pressed\n
        \t-s, --host-script <PATH>\tUse this script to gather hosts 
        info\n
        \t-w, --wifi-script <PATH>\tUse this script to gather WiFi 
        info\n
        \t-p, --port-script <PATH>\tUse this script to gather port 
        info\n
        \t-n, --wan-script <PATH>\t\tUse this script to gather WAN speed 
        and internet connection info\n
        \t-i, --basic-info-script <PATH>\tUse this script to gather 
        weather info\n
        \t-e, --weather-script <PATH>\tUse this script to gather 
        basic info\n
        \t-l, --left-long-script <PATH>\tThis script is called on long press of left key\n
        \t-y, --right-long-script <PATH>\tThis script is called on long press of right key\n
        \nThe defaults are /lib/k3screenctrl/{host,wifi,port,wan,basic,weather,left_long,right_long}.sh 
        with an interval of 2 seconds\n
```
        
# Scripts outputs

## basic.sh
This script will be called in order to get basic info
such as HW/SW version, MAC address, model etc.

Expected output format (one line for each field):
```
MODEL
HW version
FW version
New FW version
MAC address

Example:
K3
A1
r1234
r4567
02:00:00:00:00:00
```

## port.sh
This script will be called in order to get ports info.

Expected output format (one line for each field):
```
LAN1 connected? (0 or 1, applies to other fields as well)
LAN2 connected?
LAN3 connected?
WAN connected?
USB connected / mounted? (up to you)

Example:
1
1
0
1
1
```

## wan.sh
This script will be called in order to get WAN speed info.

Expected output format (one line for each field):
```
Internet connected? (0 or 1)
Upload speed (integer, in Bytes per sec)
Download speed (integer, in Bytes per sec)

Example:
1
10240000
2048000
```

## wifi.sh
This script will be called in order to get WiFi info.

Expected output format (one line for each field):
```
Does 2.4GHz and 5GHz have same SSID? (Band steering?) (0 or 1)
2.4GHz SSID
2.4GHz password (or ******* if you like, applies to other fields)
2.4GHz enabled (0 or 1)
Number of clients connected to 2.4GHz
5GHz SSID
5GHz password
5GHz enabled
Number of clients connected to 5GHz
Visitor network SSID
Visitor network password
Visitor network enabled
Number of clients connected to visitor network

Example:
0
LEDE-24G
password24
1
0
LEDE-5G
password5
1
4
<empty line>
<empty line>
0
0
```

## host.sh
This script will be called in order to get host info.

Expected output format (one line for each field):
```
Number of hosts
Host1 name
Host1 upload speed
Host1 download speed
Host1 brand (0~29)
<repetition of Host1 fields>

Example:
2
MyHost1
248193
1024000
25
MyHost2
902831
10485760
0
```

## weather.sh
This script will be called in order to get weather info.

Expected output format (one line for each field):
```
city
temp
date
time
weather
week
error

Example:
深圳市d
11
2019-02-20
14:29
25
0
0
```

## left_long.sh
This script will be called when left key is long pressed. 

The current page number is passed as a parameter for this script, thus users could define different actions on every page.
In the script `$1` represent this parameter.


## right_long.sh
This script will be called when right key is long pressed.


# Installation
1. Copy the files to the correct location, the file structure should be:
```
├── /        
    ├── usr
          ├── bin
                ├── k3screenctl
    ├── lib
          ├── k3screenctrl
                ├── working scripts
          ├── k3screenctrl-test
                ├── testing scripts
```
2. Run `k3screencctl` on foreground to test your scripts.
3. You may need to write a `init.d` script to make it run on startup, but most customized firmware has the script build-in.

# How does it work
1. Phicomm K3 has a build in PIC MCU to control the display. the main board exchanges data with the MCU to update the displaying content.
2. The MCU has some build-in templates for displaying different kinds of data. with these templates, only a few data is used for communication.
For example, if we want to update the weather picture, only a number is sent, not a full picture.
3. To exchanging data between the main board and MCU, a few protocol with data format defined is used. the communition is build on serial port.
4. [updating](https://github.com/updateing/k3screenctrl) managed to decode the protocol, then rewrite this program to replace the original one
provided by Phicomm.
5. The protocol of different versions of MCU may changed by Phicomm, which makei it difficult to be compatible with all MCU versions, 
thus we need the MCU version to match this program. 

# Build
If you don't have cross compile environment setup, you may do it on K3 directly!

1. Install gcc and a few depedencies *to the ram*: `opkg install gcc perl git make automake autoconf -d ram`
2. Add some environment to `/root/.profile`:
```
export PATH="$PATH:/tmp/bin:/tmp/sbin:/tmp/usr/bin"
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/tmp/lib:/tmp/usr/lib"

export CPATH="$CPATH:/tmp/usr/include"
```
3. Clone the repo, go the the directory, then `./configure` and `./make`, fix the errors
4. Find the binary file in `src/k3screenctl`, enjoy!
