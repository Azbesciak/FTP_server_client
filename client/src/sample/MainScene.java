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
       // files.setEditable(true);
       // files.setCellFactory(TextFieldTreeCell.forTreeView());
        files.setRoot(getRootDir());
        String files2 = "Phome/32"+(char)3+"Pcache/15"+(char)3+"Ffolder/16"+(char)3+"Pplik/10"+(char)3;
        serverFiles.setRoot(initServerFiles(files2));
        Connection connection;
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

    public String inverse(String text)
    {
        String capitalizedText="";
        boolean capitalize=false;
        for(int i=0;i<text.length();i++)
        {
            if(capitalize==true)
            {
                capitalizedText+=Character.toUpperCase(text.charAt(i));
                capitalize=false;
            }
            else
            {
                capitalizedText+=text.charAt(i);
            }
            if(text.charAt(i)==' ') capitalize=true;
        }
        capitalizedText=StringUtils.capitalize(capitalizedText);
        return capitalizedText;
    }

    @FXML
    private void connectWithServer() {

        connection = new Connection();
        Thread thread = new Thread(connection);
        thread.start();
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

    private TreeItemExtended<String> initServerFiles(String input)
    {
        TreeItemExtended<String>root = new TreeItemExtended<>(" / ");

         String k ="";
         k+=(char)3;
        String[] files = input.split(k);

        for (String file : files)
        {
            TreeItemExtended<String> treeNode;
            if(file.charAt(0)=='P')
            {
                treeNode = new TreeItemExtended<>(file);
                root.getChildren().add(treeNode);

            }
            if(file.charAt(0)=='F')
            {
                treeNode = new TreeItemExtended<>(file);
               // treeNode.getChildren().add(new TreeItem<>(""));
                root.getChildren().add(treeNode);

            }

        }
        return root;
    }

    private void getDirectory()  {

            files.getSelectionModel().selectedItemProperty().addListener(new ChangeListener() {

                @Override
                public void changed(ObservableValue observable, Object oldValue,
                                    Object newValue) {

                    TreeItem file = (TreeItem) newValue;
                    fileName.setText(file.getValue().toString());
                    // do what ever you want
                    upload.setDisable(true);
                    download.setDisable(false);
                }

            });

            serverFiles.getSelectionModel().selectedItemProperty().addListener(new ChangeListener() {

                @Override
                public void changed(ObservableValue observable, Object oldValue,
                                    Object newValue) {

                    TreeItem file = (TreeItem) newValue;
                    fileName.setText(file.getValue().toString());
                    // do what ever you want
                    download.setDisable(true);
                    upload.setDisable(false);
                }

            });



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
