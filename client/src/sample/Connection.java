package sample;

import javafx.fxml.FXML;
import javafx.scene.control.TextField;
import javafx.scene.control.TreeView;

import java.io.*;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

import static java.lang.Thread.sleep;

public class Connection implements  Runnable {

    public Socket client;
    public OutputStream out;
    public InputStream in;
    public String addr;
    public String message;
    public boolean mutex;
    private int port;
    private  PrintWriter writer;
    private BufferedReader reader;
    public String command;
    public String argument;

    public Connection(String addr,String port)
    {
        this.addr=addr;
        this.port=Integer.valueOf(port);
    }
    @Override
    public void run() {


        try {
            if (command=="CONNECT")
            {
                connect();
            }
            if(command=="LIST")
            {
                System.out.println("LIST "+argument);
                message=list();
            }

            if(command=="CWD")
            {
                cwd();
            }
//
        } catch (Exception e) {
            System.out.println("Brak połączenia");
        }
    }

//            while (keepAlive) {
//                if (in.available() > 0) {
//                    boolean binaryMode = false;
//                   // BufferedReader reader = new BufferedReader(new InputStreamReader(in));
//
//                    //String serverMessage = reader.readLine();
//                    //  System.out.println("bytes received " + in.available());
//                    //  System.out.println("data received " + serverMessage);
//
//
//                    Arrays.fill(buff, (byte) 0);
//                    in.read(buff);
//
//                    if (buff[0] == (byte) -1) {
//                        System.out.println("Server aborted connection");
//                        keepAlive = false;
//                        continue;
//                    }
//
//                    if ((int)buff[0] == 0)
//                    {
//                        System.out.println("Saving stream");
//                        stream.write(buff);
//                        break;
//                    }
//
//                    System.out.println("Received data: " + new String(buff, StandardCharsets.UTF_8));
//
//
//                }
//                sleep(10);
//            }
//
//            stream.close();
//            client.close();
//
//        } catch (IOException e) {
//            System.out.println("Brak połączenia");
//        } catch (InterruptedException e) {
//            e.printStackTrace();
//        }
//
//
//        try {
//
//        } catch (RuntimeException e) {
//            System.out.println(e.getMessage());
//        }


    public void connect () throws IOException {
        client = new Socket(addr, port);
        out = client.getOutputStream();
        in = client.getInputStream();
        writer = new PrintWriter(out, true);
        reader = new BufferedReader(new InputStreamReader(in));
    }

    public String list() throws IOException {
        String serverMessage="";
            String command = "LIST /"+argument;
            writer.println(command);
            serverMessage = reader.readLine();
        return serverMessage;

    }

    public void cwd() throws IOException {
        String serverMessage="";
        String command = "CWD "+argument;
        System.out.println(command);
        writer.println(command);
        serverMessage = reader.readLine();
        System.out.println(serverMessage);
    }

}


