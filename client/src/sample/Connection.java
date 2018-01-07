package sample;

import com.sun.xml.internal.ws.policy.privateutil.PolicyUtils;
import javafx.fxml.FXML;
import javafx.scene.control.TextField;
import javafx.scene.control.TreeItem;
import javafx.scene.control.TreeView;
import sun.reflect.generics.tree.Tree;

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
    public  PrintWriter writer;
    public BufferedReader reader;
    public String command;
    public String argument;
    public TreeItem<File> fileToUpload;
    public Socket mainSocket;
    public TreeItem fileToDownload;
    public TreeItem destinationFolder;

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
            if(command=="RMD")
            {
                rmd();
            }
            if(command=="MKD")
            {
                mkd();
            }
            if(command=="PASV")
            {
                pasv();
            }
            if(command=="MODE")
            {
                setTransmissionMode();
            }
            if(command=="STOR")
            {
                stor(fileToUpload);

            }
            if(command=="RETR")
            {
                retr(fileToDownload);
            }


//
        } catch (Exception e) {

            System.out.println(e.getMessage());
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


    public void connect ()   {
        try {
            client = new Socket(addr, port);
            out = client.getOutputStream();
            in = client.getInputStream();
            writer = new PrintWriter(out, true);
            reader = new BufferedReader(new InputStreamReader(in));
        }
        catch (IOException e)
        {
            System.out.println(e.getMessage());

        }
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

    public void rmd() throws  IOException{
        String command="RMD "+argument;
        writer.println(command);
        message = reader.readLine();
    }

    public void mkd() throws IOException{
        String command="MKD "+argument;
        writer.println(command);
        message = reader.readLine();
    }

    public void pasv() throws IOException{
        String command="PASV";
        writer.println(command);
        message = reader.readLine();
    }

    public void pwd() throws IOException{
        String command="PWD";
        writer.println(command);
        message = reader.readLine();
    }

    public void setTransmissionMode() throws IOException{
        String command="TYPE ";
        if(argument=="ASCII"){
            command+="A";
        }
        else
        {
            command+="I";
        }
        writer.println(command);
        message = reader.readLine();
    }
    public void stor(TreeItem<File> f) throws IOException{
        out = mainSocket.getOutputStream();
        in = mainSocket.getInputStream();
        writer = new PrintWriter(out, true);
        reader = new BufferedReader(new InputStreamReader(in));
        if(f.getValue().isDirectory()==true)
        {
            //stworz folder i wejdź w niego
            argument = getFileName(f);
            mkd();
            cwd();
            f.getChildren().removeAll();
            f.getChildren().clear();
            File[] listOfFiles = f.getValue().listFiles();
            if (listOfFiles != null) {
                for (File file : listOfFiles) {
                    f.getChildren().add(new TreeItem<File>(file));
                }
            }
            for(TreeItem file : f.getChildren())
            {
                stor(file);
            }
            //po przetworzeniu wszystkich dzieci wejdź na serwerze poziom wyżej
            pwd();
            String pom[] = message.split("/");
            String dir="";
            for(int i=0;i<pom.length-1;i++)
            {
                dir+=pom[i]+"/";
            }
            argument = "/";
            cwd();
            argument = dir;
            cwd();
        }
        else
        {

            if(client.isClosed())
            {
                client = new Socket(addr,port);
            }
            command = "STOR " + getFileName(f);
            writer.println(command);
            message = reader.readLine();
            DataOutputStream dos = new DataOutputStream(client.getOutputStream());
            FileInputStream fis = new FileInputStream(f.getValue());
            byte[] buffer = new byte[4096];
            while (fis.read(buffer) > 0) {
                dos.write(buffer);
            }
            fis.close();
            dos.close();
            message = reader.readLine();
        }

    }

    public void retr(TreeItem<String> f) throws IOException {
        out = mainSocket.getOutputStream();
        in = mainSocket.getInputStream();
        writer = new PrintWriter(out, true);
        reader = new BufferedReader(new InputStreamReader(in));
        if(f.getChildren().size()!=0)
        {
            File folder = new File(destinationFolder+getFileName(f));
            folder.mkdir();
            destinationFolder.setValue(folder);
            if(f.getChildren()==null)
            {
                argument = getFileName(f);
                String input = list();
                String k ="";
                k+=(char)3;
                String[] files = input.split(k);

                for (String file : files)
                {
                    TreeItemExtended<String> treeNode = new TreeItemExtended<>(file);
                    f.getChildren().add(treeNode);
                }
            }

            for(TreeItem file: f.getChildren())
            {
                retr(file);
            }


        }
        else
        {
            if(client.isClosed())
            {
                client = new Socket(addr,port);
            }
            command = "RETR " + getFileName(f);
            writer.println(command);
            message = reader.readLine();
            String path = destinationFolder.getValue().toString()+getFileName(f);
            File downloadFile = new File(path);
            OutputStream outputStream = new BufferedOutputStream(new FileOutputStream(downloadFile));
            InputStream is = client.getInputStream();
            byte[] buffer = new byte[1];
            int bytesRead = -1;

            while ((bytesRead = is.read(buffer)) != -1) {
                outputStream.write(buffer);
            }

            outputStream.close();
            is.close();
            message = reader.readLine();
        }
        }



//        destinationFolder.getChildren().removeAll();
//        destinationFolder.getChildren().clear();



    public String getFileName(TreeItem file)
    {
        String path= file.getValue().toString().replace("\\","/");
        System.out.println("aa");
        String pom[]=path.split("/");
        String name = pom[pom.length-1];
        return name;
    }

}


