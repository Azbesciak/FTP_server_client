package sample;

import javafx.fxml.FXML;
import javafx.scene.control.*;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.nio.file.*;


public class MainScene {
    @FXML
    private TreeView files;
    @FXML
    private Button connect;
    @FXML
    private Label connectionStatus;
    @FXML


    private void initialize()
    {
        listFiles();
    }


    public MainScene()
    {

    }

    @FXML
    private void connectWithServer() {
        Socket client = null;
        try {
            client = new Socket("150.254.32.67", 10001);
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
                System.out.println(in.available());
                in.read(buff);
             //   connectionStatus.setText("OK");
            }else
             //   connectionStatus.setText("Brak połączenia");

            client.close();

        } catch (IOException e) {
            connectionStatus.setText("Brak połączenia");
            e.printStackTrace();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        try{
            listDirectoryFiles("test");
        }catch(RuntimeException e)
        {
            System.out.println(e.getMessage());
        }
    }

    public final String rootDir = "ftp_client_root_dir\\";

    private void listDirectoryFiles(String directory)
            throws RuntimeException
    {

        File dir = new File(rootDir + directory);
        System.out.println(dir.getAbsolutePath());
        if(!dir.exists() || !dir.isDirectory())
            throw new RuntimeException("Podana ścieżka nie jest folderem");
        Path path = Paths.get(dir.getAbsolutePath());
        try (DirectoryStream<Path> stream = Files.newDirectoryStream(path)) {
            for (Path file: stream) {
                System.out.println(file.getFileName());
            }
        } catch (IOException | DirectoryIteratorException x) {
            // IOException can never be thrown by the iteration.
            // In this snippet, it can only be thrown by newDirectoryStream.
            System.err.println(x);
        }
    }

    private void listFiles()
    {

        files.setRoot(getRoot());
    }



    private TreeItem<String> getRoot()
    {
        TreeItem<String> root = new TreeItem<String>("Root Node");
        root.setExpanded(true);
        root.getChildren().addAll(
                new TreeItem<String>("Item 1"),
                new TreeItem<String>("Item 2"),
                new TreeItem<String>("Item 3"));
        return root;
    }




}
