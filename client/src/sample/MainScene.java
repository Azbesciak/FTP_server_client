package sample;

import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.collections.ObservableList;
import javafx.event.Event;
import javafx.fxml.FXML;
import javafx.scene.control.*;
import javafx.event.EventHandler;;
import javafx.scene.input.MouseEvent;
import java.io.*;
import java.nio.file.*;


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
    public RadioButton ascii;
    public RadioButton binary;
    private ToggleGroup group;
    @FXML
    private Button connect;
    private TreeItem<File> selectedLocalFile;
    private TreeItemExtended selectedRemoteFile;
    public TextField LocalPath;
    public TreeItem destinationFolder;
    private Connection connection;
    private Connection transferConnection;

    @FXML
    private Label connectionStatus;
    @FXML


    private void initialize()
    {
        files.setRoot(getRootDir());
        group=new ToggleGroup();
        binary.setToggleGroup(group);
        binary.setUserData("BINARY");
        ascii.setToggleGroup(group);
        ascii.setUserData("ASCII");
        listFiles();
        getDirectory();
    }


    public MainScene()
    {

    }


    @FXML
    private void connectWithServer()   {

        try {
            connection = new Connection(serverAddress.getText(), portNumber.getText());
            connection.command = "CONNECT";
            connection.connect();
            validate();
            establishTrasnferConnection();
            validate();
            connection.command = "LIST";
            connection.argument = "";
            Thread thread = new Thread(connection);
            thread.start();
            thread.join();
            serverFiles.setRoot(initServerFiles(connection.message));
            activeNode = (TreeItemExtended) serverFiles.getRoot();
            serverFiles.getRoot().addEventHandler(TreeItemExtended.branchExpandedEvent(), new EventHandler() {

                public void handle(Event e) {
                    try {
                        TreeItemExtended<String> expanded = (TreeItemExtended<String>) e.getSource();
                        if (!expanded.isLeaf()) {

                            if ((expanded != serverFiles.getRoot()) && (expanded.getParent() == activeNode)) {
                                expanded.getChildren().removeAll();
                                listServerFiles(expanded);
                                connection.command = "CWD";
                                connection.argument = "/" + getDir(expanded) + expanded.getValue();
                                System.out.println("TERAZ: " + connection.argument);
                                Thread thread = new Thread(connection);
                                thread.start();

                                thread.join();
                                activeNode = expanded;
                                RemotePath.setText(connection.argument + "/");


                            } else if (expanded != serverFiles.getRoot()) {
                                connection.command = "CWD";

                                connection.argument = "/" + getDir(expanded);
                                System.out.println("Sciezka: " + connection.argument);
                                RemotePath.setText(connection.argument + expanded.getValue() + "/");
                                Thread thread = new Thread(connection);
                                thread.start();

                                thread.join();
                                //activeNode = (TreeItemExtended) expanded.getParent();
                                System.out.println(activeNode);
                                listServerFiles(expanded);
                                connection.command = "CWD";
                                connection.argument = expanded.getValue().toString();
                                thread = new Thread(connection);
                                thread.start();
                                thread.join();
                                activeNode = expanded;


                            } else if (expanded == serverFiles.getRoot()) {
                                connection.command = "CWD";
                                connection.argument = "";
                                Thread thread = new Thread(connection);
                                thread.start();
                                thread.join();
                                activeNode = expanded;
                                System.out.println(activeNode);
                                listServerFiles(expanded);
                                RemotePath.setText("/");


                            }
                        }

                        System.out.println(expanded.getValue());

                    } catch (InterruptedException e2) {
                        //  e2.printStackTrace();
                    }

                }
            });


        }

        catch (IOException e1)
        {
            System.out.println("Brak polaczenia");
        }
        catch (InterruptedException e2)
        {
            System.out.println("Błąd systemu");
        }
    }



    public void validate() throws IOException {
        if(connection.message =="ERROR")
        {
            throw new IOException();
        }
    }
    public void removeServerDir() throws InterruptedException {
        String dir = RemotePath.getText();
        System.out.println(dir);
        connection.command="RMD";
        connection.argument=dir;
        Thread thread = new Thread(connection);
        thread.start();
        thread.join();
        if(connection.message.charAt(0)=='2') {
            activeNode.getParent().setExpanded(false);
            activeNode.getParent().setExpanded(true);
        }
    }
    @FXML
    public void makeServerDir() throws  InterruptedException{
        String dir = RemotePath.getText();
        connection.command="MKD";
        connection.argument=dir;
        Thread thread = new Thread(connection);
        thread.start();
        thread.join();
        if(connection.message.charAt(0)=='2') {
            activeNode.setExpanded(false);
            activeNode.setExpanded(true);
        }
    }

    public void upload() throws InterruptedException, IOException {
        chooseTransferMode();
        transferConnection.fileToUpload = selectedLocalFile;
        transferConnection.command="STOR";
        Thread thread = new Thread(transferConnection);
        thread.start();
        thread.join();
        activeNode.setExpanded(false);
        activeNode.setExpanded(true);
    }

    public void download() throws IOException, InterruptedException {
        chooseTransferMode();
        transferConnection.fileToDownload = selectedRemoteFile;
        transferConnection.command="RETR";
        transferConnection.argument=fileName.getText();
        transferConnection.destinationFolder = destinationFolder.getValue().toString();
        Thread thread = new Thread(transferConnection);
        thread.start();
        thread.join();
        listFolder(destinationFolder);


    }

    public void chooseTransferMode() throws IOException {
        connection.command="MODE";
        connection.argument=group.getSelectedToggle().getUserData().toString();
        connection.setTransmissionMode();
    }




    public void passiveModePort() throws InterruptedException {
        connection.command="PASV";
        Thread thread = new Thread(connection);
        thread.start();
        thread.join();
//        String input[] = connection.message.split(" ");
//        String pom[] = input[1].split(",");
//        Integer p1= Integer.valueOf(pom[0]);
//        Integer p2= Integer.valueOf(pom[1].substring(0,pom[1].length()-1));
//        Integer port = p1*256+p2;
       // return port.toString();

    }

    public void establishTrasnferConnection() throws InterruptedException, IOException {
        //String port = passiveModePort();
        passiveModePort();
        String port = "10002";
        String addr = connection.addr;
        transferConnection  = new Connection(addr,port);
        transferConnection.mainSocket = connection.client;
        transferConnection.connect();
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
                listFolder(expanded);
                }
        });

    }

    public void listFolder(TreeItem<File> expanded)
    {
        if(!expanded.isLeaf())
        {
            LocalPath.setText(expanded.getValue().toString());
            if(expanded.getValue().toString()=="Local Computer")
            {
                LocalPath.setText("\\");
            }
            else {
                expanded.getChildren().removeAll();
                expanded.getChildren().clear();
                File listOfFiles[] = expanded.getValue().listFiles();
                if (listOfFiles != null) {
                    for (File file2 : listOfFiles) {
                        expanded.getChildren().add(new TreeItem<File>(file2));
                    }
                }
            }
            for(TreeItem<File> f : expanded.getChildren())
            {
                f.getChildren().removeAll();
                f.getChildren().clear();
                File listOfFiles2[] = f.getValue().listFiles();
                if (listOfFiles2 != null) {
                    for (File file2 : listOfFiles2) {
                        f.getChildren().add(new TreeItem<File>(file2));
                    }
                }

            }
            destinationFolder = expanded;
        }
    }

    private void getDirectory()  {

            files.getSelectionModel().selectedItemProperty().addListener(new ChangeListener() {

                @Override
                public void changed(ObservableValue observable, Object oldValue,
                                    Object newValue) {

                    TreeItem<File> file = (TreeItem) newValue;
                    fileName.setText(file.getValue().toString());
                    System.out.println(file.getValue().toString());
                    upload.setDisable(false);
                    download.setDisable(true);
                    selectedLocalFile = file;
                }

            });

        serverFiles.getSelectionModel().selectedItemProperty().addListener(new ChangeListener() {

            @Override
            public void changed(ObservableValue observable, Object oldValue,
                                Object newValue) {

                TreeItemExtended file = (TreeItemExtended) newValue;
                if(file!=serverFiles.getRoot() && file.getValue()!=null) {
                    fileName.setText("/" + getDir(file) + file.getValue().toString());
                    upload.setDisable(true);
                    download.setDisable(false);
                    selectedRemoteFile = file;
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
