package sample;

import com.sun.xml.internal.ws.util.StringUtils;
import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.collections.ObservableList;
import javafx.event.Event;
import javafx.fxml.FXML;
import javafx.scene.control.*;
import javafx.event.EventHandler;
import javafx.scene.control.cell.TextFieldTreeCell;
import javafx.scene.input.MouseEvent;

import javax.swing.event.TreeWillExpandListener;
import java.io.*;
import java.lang.reflect.Array;
import java.net.Socket;
import java.net.SocketException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.TimeUnit;


public class MainScene {
    public TextField serverAddress;
    public TextField portNumber;
    public TreeItemExtended activeNode;
    @FXML
    public TreeView files;
    public TextField RemotePath;
    public Button addRemoteFolder;
    public Button deleteRemoteFolder;
    public Button download;
    public Button upload;
    public Button deleteLocalFolder;
    public Button addLocalFolder;
    public TextField fileName;
    public TreeView serverFiles;
    @FXML
    private Button connect;
    public TextField LocalPath;
    private Connection connection;
    @FXML
    private Label connectionStatus;
    @FXML


    private void initialize()
    {
        files.setRoot(getRootDir());
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
        getDirectory();
    }


    public MainScene()
    {

    }


    @FXML
    private void connectWithServer() throws InterruptedException {

        connection = new Connection(serverAddress.getText(),portNumber.getText());
        connection.command="CONNECT";
        Thread thread = new Thread(connection);
        thread.start();
        thread.join();
        connection.command="LIST";
        connection.argument="";
        thread = new Thread(connection);
        thread.start();
        thread.join();
        serverFiles.setRoot(initServerFiles(connection.message));
        activeNode= (TreeItemExtended) serverFiles.getRoot();
        serverFiles.getRoot().addEventHandler(TreeItemExtended.branchExpandedEvent(), new EventHandler() {

            public void handle(Event e) {
                try
                {
                TreeItemExtended<String> expanded= (TreeItemExtended<String>) e.getSource();
                if(!expanded.isLeaf()) {

                    if ((expanded!=serverFiles.getRoot()) && (expanded.getParent() == activeNode)) {
                        expanded.getChildren().removeAll();
                            listServerFiles(expanded);
                            connection.command = "CWD";
                            connection.argument ="/"+  getDir(expanded) + expanded.getValue();
                        System.out.println("TERAZ: "+connection.argument);
                        Thread thread = new Thread(connection);
                        thread.start();

                        thread.join();
                            activeNode=expanded;
                            RemotePath.setText(connection.argument+"/");


                    }


                    else if (expanded != serverFiles.getRoot()) {
                        connection.command = "CWD";

                        connection.argument = "/" + getDir(expanded);
                        System.out.println("Sciezka: " + connection.argument);
                        RemotePath.setText(connection.argument+expanded.getValue()+"/");
                        Thread thread = new Thread(connection);
                        thread.start();

                            thread.join();
                            activeNode = (TreeItemExtended) expanded.getParent();
                            System.out.println(activeNode);
                            listServerFiles(expanded);



                    } else if (expanded == serverFiles.getRoot()) {
                        connection.command = "CWD";
                        connection.argument = "/";
                        Thread thread = new Thread(connection);
                        thread.start();
                            thread.join();
                            activeNode = (TreeItemExtended) serverFiles.getRoot();
                            System.out.println(activeNode);
                            listServerFiles(expanded);
                            RemotePath.setText("/");


                    }
                }

                    System.out.println(expanded.getValue());

                }
                catch (InterruptedException e2)
                {
                    e2.printStackTrace();
                }

            }
        });
    }


    private TreeItemExtended<String> initServerFiles(String input)
    {
       // String input = connection.LIST("/");
        TreeItemExtended<String>root = new TreeItemExtended<>("F /0");
        root.getChildren().clear();


        String k ="";
        k+=(char)3;
        String[] files = input.split(k);

        for (String file : files)
        {
            TreeItemExtended<String> treeNode = new TreeItemExtended<>(file);
            root.getChildren().add(treeNode);
        }
        return root;
    }

    public void listServerFiles(TreeItemExtended parent) throws InterruptedException {
        connection.command="LIST";
        connection.argument=parent.getValue().toString()+"/";
        Thread thread = new Thread(connection);
        thread.start();
        thread.join();
        String k ="";
        k+=(char)3;
        String[] files = connection.message.split(k);
        parent.getChildren().removeAll();
        parent.getChildren().clear();
        for (String file : files)
        {
            TreeItemExtended<String> treeNode = new TreeItemExtended<>(file);
            parent.getChildren().add(treeNode);
            serverFiles.refresh();

        }


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
                            f.getChildren().add(new TreeItem<File>(file));
                        }
                    }
                }
            }
        });

    }

    private void getDirectory()  {

            files.getSelectionModel().selectedItemProperty().addListener(new ChangeListener() {

                @Override
                public void changed(ObservableValue observable, Object oldValue,
                                    Object newValue) {

                    TreeItem file = (TreeItem) newValue;
                    fileName.setText(file.getValue().toString());
                    upload.setDisable(true);
                    download.setDisable(false);
                }

            });

        serverFiles.getSelectionModel().selectedItemProperty().addListener(new ChangeListener() {

            @Override
            public void changed(ObservableValue observable, Object oldValue,
                                Object newValue) {

                TreeItemExtended file = (TreeItemExtended) newValue;
                if(file!=serverFiles.getRoot()) {
                    fileName.setText("/" + getDir(file) + file.getValue().toString());
                    upload.setDisable(false);
                    download.setDisable(true);
                }
            }

        });
        
    }

    public String getDir(TreeItemExtended file)
    {
        String path="";
        while (file.getParent() != serverFiles.getRoot()) {
            String dir = file.getParent().getValue().toString() + "/";
            path = dir + path;
            file = (TreeItemExtended) file.getParent();
        }
        return path;
    }



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
