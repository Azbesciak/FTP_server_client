package sample;

import javafx.collections.ObservableList;
import javafx.event.Event;
import javafx.fxml.FXML;
import javafx.scene.control.*;
import javafx.event.EventHandler;
import javafx.scene.control.cell.TextFieldTreeCell;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.nio.file.*;
import java.util.List;


public class MainScene {
    public TextField serverAddress;
    public TextField portNumber;
    @FXML
    public TreeView files;
    @FXML
    private Button connect;
    @FXML
    private Label connectionStatus;
    @FXML


    private void initialize()
    {
       // files.setEditable(true);
       // files.setCellFactory(TextFieldTreeCell.forTreeView());
        files.setRoot(getRootDir());
        listFiles();
    }


    public MainScene()
    {

    }

    @FXML
    private void connectWithServer() {
        Socket client = null;
        try {
            //client = new Socket(serverAddress.getText() , Integer.valueOf(portNumber.getText()));
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

    private void listFiles() {

        files.getRoot().addEventHandler(TreeItem.branchExpandedEvent(), new EventHandler() {

            public void handle(Event e) {
                TreeItem<File> expanded= (TreeItem<File>) e.getSource();
                ObservableList<TreeItem<File>> files = expanded.getChildren();

                for (TreeItem<File> f : files) {
                    File[] listOfFiles = f.getValue().listFiles();
                    if (listOfFiles != null) {
                        for (File file : listOfFiles) {
                            //   if (file.isFile()) {
                          //  TreeItem<File>fil = new TreeItem<File>(file);
                          //  fil.valueProperty().;
                            f.getChildren().add(new TreeItem<File>(file));
                            //  }
                        }
                    }
                }
            }
        });
    }



    private TreeItem<File> getRootDir()
    {
        TreeItem<File> root = new TreeItem<File>(new File("LocalComputer"));
        Iterable<Path> rootDirectories=FileSystems.getDefault().getRootDirectories();
        for(Path name:rootDirectories) {

            TreeItem<File> treeNode = new TreeItem<File>(new File(name.toString()));

            root.getChildren().add(treeNode);
        }
         root.setExpanded(false);
        return root;
    }
}
