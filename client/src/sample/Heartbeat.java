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


    public Heartbeat(OutputStream out,InputStream in)
    {
        this.out=out;
        this.in=in;
        writer = new PrintWriter(out, true);
        reader = new BufferedReader(new InputStreamReader(in));
        message="";
    }
    public void  run(){
        while(true)
        {
            isConnected=false;
            writer.println("PWD");
            try {
                message =(reader.readLine());
                isConnected=true;
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
