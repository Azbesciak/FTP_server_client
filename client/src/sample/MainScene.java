package sample;

import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.collections.ObservableList;
import javafx.event.Event;
import javafx.fxml.FXML;
import javafx.scene.control.*;
import javafx.event.EventHandler;
import javafx.scene.control.cell.TextFieldTreeCell;
import javafx.scene.input.MouseEvent;

import java.io.*;
import java.lang.reflect.Array;
import java.net.Socket;
import java.net.SocketException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.Arrays;
import java.util.List;


public class MainScene {
    public TextField serverAddress;
    public TextField portNumber;
    @FXML
    public TreeView files;
    public TextField RemotePath;
    public Button addRemoteFolder;
    public Button deleteRemoteFolder;
    public Button download;
    public Button upload;
    public Button deleteLocalFolder;
    public Button addLocalFolder;
    @FXML
    private Button connect;
    public TextField LocalPath;
    private Connection connection;
    @FXML
    private Label connectionStatus;
    @FXML


    private void initialize()
    {
       // files.setEditable(true);
       // files.setCellFactory(TextFieldTreeCell.forTreeView());
        files.setRoot(getRootDir());
        connection= new Connection();
        files.setOnMouseClicked(new EventHandler<MouseEvent>() {
            @Override
            public void handle(MouseEvent event) {
                if(event.getClickCount() == 2)
                {
                    ObservableList<TreeItem<File>> file = files.getSelectionModel().getSelectedItems();

                    System.out.println(file.get(0).getValue());
                }
            }
        });
        listFiles();
      //  getDirectory();
    }


    public MainScene()
    {

    }

    @FXML
    private void connectWithServer() {
        connection.run();
    }


    public final String rootDir = "ftp_client_root_dir\\";

    private void listDirectoryFiles(String directory)
            throws RuntimeException {

        File dir = new File(rootDir + directory);
        System.out.println(dir.getAbsolutePath());
        if (!dir.exists() || !dir.isDirectory())
            throw new RuntimeException("Podana ścieżka nie jest folderem");
        Path path = Paths.get(dir.getAbsolutePath());
        try (DirectoryStream<Path> stream = Files.newDirectoryStream(path)) {
            for (Path file : stream) {
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
                if(!expanded.isLeaf())
                {
                    LocalPath.setText(expanded.getValue().toString());
                   if(expanded.getValue().toString()=="Local Computer")
                    {
                        LocalPath.setText("\\");
                    }

                }
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

//    private void getDirectory() {
//        files.getSelectionModel().selectedItemProperty().addListener( new ChangeListener() {
//
//            @Override
//            public void changed(ObservableValue observable, Object oldValue,
//                                Object newValue) {
//
//                TreeItem<String> selectedItem = (TreeItem<String>) newValue;
//                System.out.println(selectedItem.getValue().toString());
//                // do what ever you want
//            }
//
//        });
//
//    }



    private TreeItem<File> getRootDir()
    {
        TreeItem<File> root = new TreeItem<File>(new File("Local Computer"));
        Iterable<Path> rootDirectories=FileSystems.getDefault().getRootDirectories();
        for(Path name:rootDirectories) {

            TreeItem<File> treeNode = new TreeItem<File>(new File(name.toString()));

            root.getChildren().add(treeNode);
        }
         root.setExpanded(false);
        return root;
    }
}
