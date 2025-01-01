# Control Monoprice HDMI Switch Model 4067 (HRM-2218F) via RS-232

## MOTIVATION

The Monoprice [HRM-2218F](https://archive.org/details/monoprice_hdmi_selector_HRM-2214F_HRM-2218F_user_manual/mode/2up)
is an automatic HDMI switch that supports RS-232 based control.
This program was written to simplify the process of automating the
HRM-2218F on Linux.

## CREDIT

The [mono-switcher](https://github.com/CelestinaDragoni/mono-switcher) program
by Anthony Mattera (KernelZechs) helped me to locate the binary protocol,
originally reverse engineered by [Josh in an Amazon review](https://www.amazon.com/gp/customer-reviews/RJFTW648Q1UIO/ref=cm_cr_dp_d_rvw_ttl?ie=UTF8&ASIN=B003L14X3A).
I have included the CREDIT.md file (a copy of Josh's Amazon review) which was
prepared by Anthony Mattera in this repo for reference.
 
## BUILD INSTRUCTIONS

- This program is intended to be built and run on Linux
- Install a C compiler if one is not present
  (Run ```sudo apt install build-essential``` on Debian distros)
- ```cd``` to this repo's directory
- Run ```make```

## SYNOPSIS

```ctrl_monoprice_4067 <DEVICE> [-i input] [-r retries] [-t ms] [-v]```

## DESCRIPTION

Simple terminal program to control the HRM-2218F.

```DEVICE``` should be the RS-232 device connected to the HRM-2218F, it will
likely be located at **/dev/tty\***.

This program may return one of three values:
 - 0 (Success)
 - 1 (Failure)
 - 2 (Invalid arguments)

On success this program will output (to stdout) the currently active HDMI input,
or zero if no input is active.

Errors and verbose messages are output to stderr.

When changing inputs the currently active input will not change if
that input is not currently active, for example:

 - Assuming HDMI **1** and **2** are active and the current input is
   **1** then running:<br>
   ```ctrl_monoprice_4067 -i 2```<br>
   Will output: **2**<br>
   However, running:<br>
   ```ctrl_monoprice_4067 -i 3```<br>
   Will output: **1**<br>

## COMMAND-LINE OPTIONS

 - **-i** *input*<br>
  Set the input, valid range is **\[1-8\]**
 - **-r** *retries*<br>
  Set the number of retries before giving up, valid range is **\[0-99\]**
 - **-t** *timeout*<br>
  Set the timeout (in milliseconds) for calls to *read()* and *write()*,
  valid range is **\[10-10000\]**
 - **-v**<br>
  Enable verbose output

## License
This program is licensed under the
[MIT License](https://opensource.org/licenses/MIT).
