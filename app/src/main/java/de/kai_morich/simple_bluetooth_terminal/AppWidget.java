package de.kai_morich.simple_bluetooth_terminal;

import android.app.PendingIntent;
import android.appwidget.AppWidgetManager;
import android.appwidget.AppWidgetProvider;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.widget.RemoteViews;
import android.widget.Toast;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import java.util.UUID;
import java.io.OutputStream;
import java.io.IOException;
import android.os.Handler;
import java.util.Calendar;


public class AppWidget extends AppWidgetProvider {
    private String newline = TextUtil.newline_crlf;


    static void updateAppWidget(Context context, AppWidgetManager appWidgetManager, int appWidgetId) {
        RemoteViews views = new RemoteViews(context.getPackageName(), R.layout.app_widget);

        // Create PendingIntent for the Light button
        Intent lightIntent = new Intent(context, AppWidget.class);
        lightIntent.setAction("TOGGLE_LIGHT");
        PendingIntent lightPendingIntent = PendingIntent.getBroadcast(
                context,
                87,
                lightIntent,
                PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE
        );
        views.setOnClickPendingIntent(R.id.toggle_light, lightPendingIntent);

        // Create PendingIntent for the Fan button
        Intent fanIntent = new Intent(context, AppWidget.class);
        fanIntent.setAction("TOGGLE_FAN");
        PendingIntent fanPendingIntent = PendingIntent.getBroadcast(
                context,
                88, // Use a unique requestCode for each PendingIntent
                fanIntent,
                PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE
        );
        views.setOnClickPendingIntent(R.id.toggle_fan, fanPendingIntent);


        // Update the widget with the new PendingIntents
        appWidgetManager.updateAppWidget(appWidgetId, views);
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        super.onReceive(context, intent);

        // Log the received intent action for debugging
        Log.d("AppWidget", "Received action: " + intent.getAction());

        // Get the current time in HHMM format
        String currentTime = getCurrentTime();

        // Check which action was triggered
        if ("TOGGLE_LIGHT".equals(intent.getAction())) {
            // Send "L" with the current time
            sendBluetoothMessage(context, "L" + currentTime);
        } else if ("TOGGLE_FAN".equals(intent.getAction())) {
            // Send "F" with the current time
            sendBluetoothMessage(context, "F" + currentTime);
        }
    }

    /**
     * Helper method to get the current time in HHMM format
     */
    private String getCurrentTime() {
        // Get the current time
        Calendar calendar = Calendar.getInstance();
        int hour = calendar.get(Calendar.HOUR_OF_DAY); // 24-hour format
        int minute = calendar.get(Calendar.MINUTE);

        // Format the time as a 4-digit string (HHMM)
        return String.format("%02d%02d", hour, minute);
    }



    private void sendBluetoothMessage(Context context, String message) {
        // The address of the ESP32 device (ensure to replace with the correct address)
        String esp32Address = "0C:8B:95:75:03:BA"; // Replace with your ESP32 device's MAC address

        // Attempt to connect to the Bluetooth device
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (bluetoothAdapter == null || !bluetoothAdapter.isEnabled()) {
            Toast.makeText(context, "Bluetooth is not enabled", Toast.LENGTH_SHORT).show();
            return;
        }

        BluetoothDevice device = bluetoothAdapter.getRemoteDevice(esp32Address);
        final BluetoothSocket[] socketWrapper = new BluetoothSocket[1];  // Wrap the socket in a final array

        try {
            // Create a BluetoothSocket using the device's MAC address
            socketWrapper[0] = device.createRfcommSocketToServiceRecord(UUID.fromString("00001101-0000-1000-8000-00805F9B34FB")); // Replace with your UUID

            // Attempt to connect to the device
            socketWrapper[0].connect();

            Log.i("AppWidget", "sendBluetoothMessage: " + socketWrapper[0].isConnected());
            Log.i("AppWidget", "sendBluetoothMessage: " + socketWrapper[0].toString());

            // Create a Handler to add a 50ms delay after connect
            Handler handler = new Handler();
            handler.postDelayed(() -> {
                try {
                    // Send the message to toggle the light or fan after 50ms delay
                    OutputStream outputStream = socketWrapper[0].getOutputStream();
                    byte[] data = (message + "\n").getBytes();
                    outputStream.write(data);
                    outputStream.flush();

                    Log.i("AppWidget", "data: " + data);

                    // Create another delay (100ms) before closing the socket
                    handler.postDelayed(() -> {
                        try {
                            // Close the socket after 100ms delay
                            socketWrapper[0].close();
                            Log.i("AppWidget", "Socket closed");
                        } catch (IOException e) {
                            Log.e("AppWidget", "Failed to close socket", e);
                        }
                    }, 50); // 100ms delay before closing the socket

                    // Show a success message
                    Toast.makeText(context, "Message sent: " + message, Toast.LENGTH_SHORT).show();
                } catch (IOException e) {
                    Log.e("AppWidget", "Failed to send message", e);
                    Toast.makeText(context, "Failed to send message", Toast.LENGTH_SHORT).show();
                }
            }, 50); // 50ms delay after socket.connect()

        } catch (IOException e) {
            Log.e("AppWidget", "Bluetooth connection failed", e);
            Toast.makeText(context, "Failed to connect", Toast.LENGTH_SHORT).show();
        }
    }





    @Override
    public void onUpdate(Context context, AppWidgetManager appWidgetManager, int[] appWidgetIds) {
        // Update all widgets when they need to be updated
        for (int appWidgetId : appWidgetIds) {
            updateAppWidget(context, appWidgetManager, appWidgetId);
        }
    }
}
