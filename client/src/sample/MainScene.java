package sample;

import javafx.fxml.FXML;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.text.Font;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.nio.Buffer;

public class MainScene {
    @FXML
    private Button connect;
    @FXML
    private Label connectionStatus;
    @FXML
    private void initialize()
    {

    }


    public MainScene()
    {

    }

    @FXML
    private void connectWithServer() {
        Socket client = null;
        try {
            client = new Socket("127.0.0.1", 10001);
            OutputStream out = client.getOutputStream();
            out.write("12;3".getBytes());
            System.out.println("wysłano");

            //waits for response
            Thread.sleep(500);


            //odpowiedz
            byte buff[]= new byte[100];
            InputStream in  = client.getInputStream();
            if(in.available() > 0)
            {
                in.read(buff);
                connectionStatus.setText("OK");
            }else
                connectionStatus.setText("Brak połączenia");

            client.close();

        } catch (IOException e) {
            connectionStatus.setText("Brak połączenia");
            e.printStackTrace();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }


    }
}
