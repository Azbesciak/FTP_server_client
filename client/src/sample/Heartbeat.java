package sample;

import java.io.*;
import java.net.SocketException;
import java.util.concurrent.TimeUnit;

public class Heartbeat implements Runnable{

    private boolean isConnected;
    public OutputStream out;
    public InputStream in;
    public PrintWriter writer;
    public BufferedReader reader;
    public String message;
    public Connection mainConnection;
    public boolean mutex;


    public Heartbeat(OutputStream out,InputStream in)
    {
        this.out=out;
        this.in=in;
        writer = new PrintWriter(out, true);
        reader = new BufferedReader(new InputStreamReader(in));
        message="";
        mutex=false;
    }
    public void  run(){
        while(true)
        {
            try {
//                while(mainConnection.transferConnection.transferGoing==true){}
//                while(mainConnection.mutex==true)
//                {}
//                while(mainConnection.transferConnection.mutex==true){}
                isConnected = false;
                mutex=true;
                writer.println("STATUS");
                message= reader.readLine();
                mutex=false;
                isConnected = true;
                TimeUnit.MILLISECONDS.sleep(1000);
            } catch(Exception e)
            {
                isConnected=false;
                return;
            }
        }
    }
    public synchronized boolean getIsConnected()
    {
        return isConnected;
    }

}
