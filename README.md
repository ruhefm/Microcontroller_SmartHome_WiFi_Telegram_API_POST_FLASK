First, upload the cpp code to MicroController.

Make sure it is connected to WiFi by configuring it in the cpp code.

In this program, I'm using teleapigps.pythonanywhere.com as the web destination.

../key is used for receiving UID and key state data from Microcontroller POST request.

../temp is used for receiving temp and humidity data from Microcontroller POST request.

The web backend used flask requests, to receive data, and then used the Telegram library as the framework to communicate with Telegram bot API.

The microcontroller uses Millis to prevent delay in all activity.

For the database, I use Supabase and PostgreSQL.
