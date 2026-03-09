# Dice to key




## DISCLAIMER!!!
This is only a fun experiment. This is an unsafe way of generating a private key for RSA cryptography.
Since this is based on dice that have a more limited amount of numbers, there is less difficulty to find the right key.
Its fun to play with the software. Maybe you can use it to create your own version. Change some things, adapt it to run new algorithms, maybe someone will one day make it reliable for real-world security, but this is not that version.

As always:

This software is provided "as is," without warranty of any kind, express or implied. In no event shall the developer be held liable for any claim, damages, or other liability, including but not limited to the loss of funds or unauthorized access to accounts, arising from the use of this application.

## Build instructions
You'll need both the boost and openssl libraries.

1.) The configure command will be cmake -S . --build build

2.) then run cmake --build build

The binary will compile into the build/Debug folder