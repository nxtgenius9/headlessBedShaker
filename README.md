# headlessBedShaker

This is a headless bed shaker driver I based on a wemos D1 mini with a few mosfets I had lying around. It projects a WAP which is utilized to set the alarm time and powers a bedshaker at the same time every day.  All you need is this device, something to power it, a bed shaker and a smart phone and you're good to go.  

## The circuit

Attached to the project, I will also upload a picture of the wiring of this device.  Since we aren't doing much with the board aside from driving this mosfet I thought I could get away with a direct hookup to the gate and I was correct.  However, the 3V3 logic of the D1 didn't jive with the 5V of the USB input so relinquishing control of the digital line and adding a pull up solved that issue.  [This](https://www.amazon.com/Vibrate-Shaker-Vibrating-Alarm-DryBuddyFLEX/dp/B07SXD783Z) is a bed shaker like the one I used for this project.  You could also get crafty and combine [these](https://www.amazon.com/uxcell-4500RPM-Torque-Vibration-Electric/dp/B01N1GAWQ5/) with 1.5" PVC and 1" end caps as well as electrical tape and hot glue.  I also had 2.5mm x 5.5mm barral jacks and plugs on hand but you'll have to get one or adapt whatever you get to it.   [This](https://www.amazon.com/Power-Connector-Female-Adapter-Camera/dp/B07C61434H) is what I am using to connect them.

## Using the system

Upon start up, you have a minute before it will assume the device is set up.  In a future update I may fix the vibration routine to not jam the web server up with delays but for me this has worked.  The device presents a webserver with your SECRET_SSID and SECRET_PASS that you can connect to.  Visiting 192.168.4.1 takes you to the mini's web server.  From there you have the clock page, the status page, and the alarm page.  

### Setting up Time

By visiting the 192.168.4.1/clock?hour=12&minute=45 you are setting the time of the device to 12:45 PM (afternoon).  Substitute the hour and minute that is currently displayed on your device to set those arguments.  If it is successful, you will get a response with those as well as what the midnightMillis value will be at midnight of that day.

### Setting up Alarm

The URL for setting up the alarm is 192.168.4.1/alarm?hour=7&minute=0&length=300 where hour is the hour you want it to go off, minute is the minute you want it to start, and length is the length of time you want it to go on for.  There are other arguments I have been working on adding but they are not functional yet.  At minimum the shaker will turn on once, and then have an off time.  They are hard coded so that I don't risk overheating my bed shaker (again).  The length doesn't have to be specified, it defaults to 300 seconds but you can change it if you'd like.


### On the Go Status Checks

In addition to the USB serial output, I also gave myself a page for viewing the status of the bed shaker driver.  visiting the /status page will show you what time the clock thinks it currently is, and when it will activate again.  I left these in raw hour counts so that the on duration can be ascertained and make sure to reset this device before day 47 rolls around.  It could work just fine but I haven't checked that yet and that's around the time when the millis() function on the D1 mini rolls over.
