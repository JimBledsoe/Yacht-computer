# Config files for Raspberry Pi 

There are aways some minor settings that can easily be overlooked. Most things comes nicely packaged with the [http://www.sailoog.com/blog-categories/openplotter-rpi](NOOB Openplotter distro).

## Lost access point 
As always, updates and upgrades are not always safe. Most major
disasters happen after an update or upgrade, where important servies
are affected. After an upgrade in May 2019, I lost the access
point, all my IoToB devices failed to connect. 
Some search at various sites and forums was needed to resolve
this. The actual fix was actually simple, just replacing a single line
in a config file.

### Script file for dnsmadq
The file /etc/init.d/dnsmasq had a line that needed to be replaced, the line 115
needed to be replaced. The actual file is presented in this directory. The original line is commented out and a working line is added.

Some web references:

- [http://forum.openmarine.net/showthread.php?tid=1083](openmarine forum, thread about lost AP).

- [https://forums.kali.org/showthread.php?38920-Access-point-configuration-problem-on-RPi3](Some hints about AP config)

- [https://www.raspberrypi.org/forums/viewtopic.php?t=128449](Failing to start dnsmasq)

- [https://discourse.pi-hole.net/t/dnsmasq-not-starting/10523/13]("dnsmasq not starting" at discourse)

The last reference is the one that actually showed the error in the dnsmasq script (as aways the devel is in the details).

