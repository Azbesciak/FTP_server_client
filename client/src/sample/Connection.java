package sample;

import javafx.scene.control.TextField;

import java.io.*;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

import static java.lang.Thread.sleep;

public class Connection implements  Runnable {
    @Override
    public void run() {
        Socket client = null;
        try {
            client = new Socket("127.0.1.1", 10001);
           // client = new Socket(serverAddress.getText() , Integer.valueOf(portNumber.getText()));
            OutputStream out = client.getOutputStream();
            InputStream in = client.getInputStream();

            //init list
            String command = "LIST /";
            PrintWriter writer = new PrintWriter(out, true);
            writer.println(command);

            BufferedReader reader = new BufferedReader(new InputStreamReader(in));
            String serverMessage = reader.readLine();
            System.out.println(serverMessage);
            FileOutputStream stream = new FileOutputStream("/");
            //odpowiedz
            byte buff[] = new byte[100];

            boolean keepAlive = true;
            while (keepAlive) {
                if (in.available() > 0) {
                    boolean binaryMode = false;
                   // BufferedReader reader = new BufferedReader(new InputStreamReader(in));

                    //String serverMessage = reader.readLine();
                    //  System.out.println("bytes received " + in.available());
                    //  System.out.println("data received " + serverMessage);


                    Arrays.fill(buff, (byte) 0);
                    in.read(buff);

                    if (buff[0] == (byte) -1) {
                        System.out.println("Server aborted connection");
                        keepAlive = false;
                        continue;
                    }

                    if ((int)buff[0] == 0)
                    {
                        System.out.println("Saving stream");
                        stream.write(buff);
                        break;
                    }

                    System.out.println("Received data: " + new String(buff, StandardCharsets.UTF_8));


                }
                sleep(10);
            }

            stream.close();
            client.close();

        } catch (IOException e) {
            System.out.println("Brak połączenia");
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


        try {

        } catch (RuntimeException e) {
            System.out.println(e.getMessage());
        }
    }
}


