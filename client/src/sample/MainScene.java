package sample;

import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.event.Event;
import javafx.fxml.FXML;
import javafx.scene.control.*;
import javafx.event.EventHandler;;
import java.io.*;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.nio.file.*;
import java.text.DecimalFormat;


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
    public Button disconnect;
    public Label sizeText;
    public CheckBox portHardCoded;
    private ToggleGroup group;
    @FXML
    private Button connect;
    private TreeItem<File> selectedLocalFile;
    private TreeItemExtended selectedRemoteFile;
    public TextField LocalPath;
    public TreeItem destinationFolder;
    private Connection connection;
    private Connection transferConnection;
    public Heartbeat heartbeat;

    @FXML
    private Label connectionStatus;

    @FXML


    private void initialize() {
        files.setRoot(getRootDir()); //zainicjuj lokalne drzewo plików
        group = new ToggleGroup();
        binary.setToggleGroup(group);
        binary.setUserData("BINARY");
        ascii.setToggleGroup(group);
        ascii.setUserData("ASCII");
        listFiles(); //dodaj obsługę przeszukiwania lokalnego drzewa
        getDirectory(); //dodaj obsługę wybierania folderu
    }


    public MainScene() {

    }


    @FXML
    private void connectWithServer() {

        try {
            disconnect();
            connection = new Connection(serverAddress.getText(), portNumber.getText());
            connection.command = "CONNECT";
            connection.connect();
            validate();
            if (checkServerCapacity() == true) return;
            establishTrasnferConnection(); //utwórz połączenie do transferu plików
            validateTransferConnection();
            connection.command = "LIST";
            connection.argument = "";
            Thread thread = new Thread(connection);
            thread.start();
            thread.join();
            heartbeat = new Heartbeat(connection.out,connection.in);
            Thread heartBeatThread = new Thread(heartbeat);
            heartBeatThread.setDaemon(true);
            heartBeatThread.start(); //wątek do sprawdzania czy serwer jest dostępny
            serverFiles.setRoot(initServerFiles(connection.message));
            activeNode = (TreeItemExtended) serverFiles.getRoot();
            //przechodzenie po plikach serwera
            serverFiles.getRoot().addEventHandler(TreeItemExtended.branchExpandedEvent(), new EventHandler() {

                public void handle(Event e) {
                    try {
                        if(checkConnecionStatus()==true)return;
                        TreeItemExtended<String> expanded = (TreeItemExtended<String>) e.getSource();
                        if (!expanded.isLeaf()) {

                            if ((expanded != serverFiles.getRoot()) && (expanded.getParent() == activeNode)) {
                                expanded.getChildren().removeAll();
                                listServerFiles(expanded);
                                connection.command = "CWD";
                                connection.argument = "/" + getDir(expanded) + expanded.getValue();
                                Thread thread = new Thread(connection);
                                thread.start();

                                thread.join();
                                activeNode = expanded;
                                RemotePath.setText(connection.argument + "/");


                            } else if (expanded != serverFiles.getRoot()) {
                                connection.command = "CWD";

                                connection.argument = "/" + getDir(expanded);
                                RemotePath.setText(connection.argument + expanded.getValue() + "/");
                                Thread thread = new Thread(connection);
                                thread.start();

                                thread.join();
                                //activeNode = (TreeItemExtended) expanded.getParent();
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
                                listServerFiles(expanded);
                                RemotePath.setText("/");


                            }
                        }


                    } catch (InterruptedException e2) {

                   }
                }
            });


        } catch (IOException e1) {
            showError("Nie nawiązano połączenia");
        } catch (InterruptedException e2) {
            showError("Błąd systemu");
        }
    }

    public void disconnect() throws IOException {
        if (transferConnection != null ) {
            if(transferConnection.client!=null) {
                transferConnection.client.close();
                transferConnection = null;
            }
        }
        if ( connection!=null  ) {
            if(connection.client!=null) {
                connection.client.close();
                connection = null;
            }
        }
    }


    //sprawdza czy Socket został utworzony
    public void validate() throws IOException {
        if (connection==null || connection.message == "ERROR") {
            throw new IOException();
        }
    }

    public void validateTransferConnection() throws IOException {
        if (transferConnection.message == "ERROR" || transferConnection==null) {
            showError("Nie nawiązano połączenia");
            throw new IOException();
        }
    }

    public boolean checkServerCapacity() {
        if (connection.message.charAt(0) == '5') {
            showError("Serwer przepełniony");
            return true;
        }
        return false;
    }

    //sprawdza czy serwer jest nadal aktywny
    public boolean checkConnecionStatus()  {
            if (connection == null) {
                showError("Brak połączenia ");
                serverFiles.setRoot(null);
                return true;

            }

            if (connection.client == null) {
                showError("Brak połączenia ");
                serverFiles.setRoot(null);
                return true;

            }
        if( heartbeat.getIsConnected() == false || heartbeat.message==null) {
            showError("Połączenie zerwane");
            serverFiles.setRoot(null);
            return true;
        }
        return false;

    }

    public void showError(String s) {
        Alert alert = new Alert(Alert.AlertType.INFORMATION);
        alert.setTitle("Błąd");
        alert.setHeaderText(null);
        alert.setContentText(s);
        alert.showAndWait();
    }

    public void removeServerDir() throws InterruptedException, IOException {

        if (checkConnecionStatus() == true) return;
        String dir = RemotePath.getText();
        connection.command = "RMD";
        connection.argument = dir;
        Thread thread = new Thread(connection);
        thread.start();
        thread.join();
        if (connection.message.charAt(0) == '2') {
            activeNode.getParent().setExpanded(false);
            activeNode.getParent().setExpanded(true);
        } else {
            showError(connection.message);
        }
    }


    @FXML
    public void makeServerDir() throws InterruptedException, IOException {
        if (checkConnecionStatus() == true) return;
        String dir = RemotePath.getText();
        connection.command = "MKD";
        connection.argument = dir;
        Thread thread = new Thread(connection);
        thread.start();
        thread.join();
        if (connection.message.charAt(0) == '2') {
            activeNode.setExpanded(false);
            activeNode.setExpanded(true);
        } else {
            showError(connection.message);
        }
    }

    public void upload() throws InterruptedException, IOException {
        if (checkConnecionStatus() == true) return;
        chooseTransferMode();
        transferConnection.fileToUpload = selectedLocalFile;
        transferConnection.command = "STOR";
        Thread thread = new Thread(transferConnection);
        thread.start();
        thread.join();
        if (transferConnection.message == "ERROR2") {
            showError("Błąd transferu danych");
            return;
        }
        activeNode.setExpanded(false);
        activeNode.setExpanded(true);
    }

    public void download() throws IOException, InterruptedException {
        if (checkConnecionStatus() == true) return;
        chooseTransferMode();
        transferConnection.fileToDownload = selectedRemoteFile;
        transferConnection.command = "RETR";
        transferConnection.argument = fileName.getText();
        transferConnection.destinationFolder = destinationFolder.getValue().toString();
        Thread thread = new Thread(transferConnection);
        thread.start();
        thread.join();
        if (transferConnection.message == "ERROR2") {
            showError("Błąd transferu danych");
            return;
        }
        listFolder(destinationFolder);


    }

    //wybiera transfer ASCII lub BINARY
    public void chooseTransferMode() throws IOException {
        connection.command = "MODE";
        connection.argument = group.getSelectedToggle().getUserData().toString();
        connection.setTransmissionMode();
    }


    //uzyskuje port na którym będą przesyłane pliki
    public String passiveModePort() throws InterruptedException, IOException {
        connection.command = "PASV";
        Thread thread = new Thread(connection);
        thread.start();
        thread.join();
        validate();
        if(portHardCoded.isSelected()==false) {
        String input[] = connection.message.split(" ");
        String pom[] = input[1].split(",");
        Integer p1 = Integer.valueOf(pom[0]);
        Integer p2 = Integer.valueOf(pom[1].substring(0, pom[1].length() - 1));
        Integer port = p1 * 256 + p2;
        return port.toString();
        }
        else return "10002";

    }

    //nazwiązuje polączenie do transferu plików
    public void establishTrasnferConnection() throws InterruptedException, IOException {
       String port = passiveModePort();
        String addr = connection.addr;
        transferConnection = new Connection(addr, port);
        transferConnection.mainSocket = connection.client;
        transferConnection.connect();
    }

    private TreeItemExtended<String> initServerFiles(String input) {
        TreeItemExtended<String> root = new TreeItemExtended<>("F /0");
        root.getChildren().clear();


        String k = "";
        k += (char) 3; //znak ASCII 3 rozdziela pliki
        String[] files = input.split(k);

        for (String file : files) {
            TreeItemExtended<String> treeNode = new TreeItemExtended<>(file);
            root.getChildren().add(treeNode);
        }
        return root;
    }

    public void listServerFiles(TreeItemExtended parent) throws InterruptedException {
        connection.command = "LIST";
        connection.argument = parent.getValue().toString();
        Thread thread = new Thread(connection);
        thread.start();
        thread.join();
        String k = "";
        k += (char) 3;
        String dataFromServer = connection.message;
        if(dataFromServer.length()>2) {
            String[] files = connection.message.split(k);
            parent.getChildren().removeAll();
            parent.getChildren().clear();
            for (String file : files) {
                TreeItemExtended<String> treeNode = new TreeItemExtended<>(file);
                parent.getChildren().add(treeNode);
                serverFiles.refresh();

            }
        }
    }


    public final String rootDir = "ftp_client_root_dir\\";

    private void listFiles() {

        files.getRoot().addEventHandler(TreeItem.branchExpandedEvent(), new EventHandler() {

            public void handle(Event e) {
                TreeItem<File> expanded = (TreeItem<File>) e.getSource();
                listFolder(expanded);
            }
        });

    }


    public void listFolder(TreeItem<File> expanded) {
        if (!expanded.isLeaf()) {
            LocalPath.setText(expanded.getValue().toString());
            if (expanded.getValue().toString() == "Local Computer") {
                LocalPath.setText("\\");
            } else {
                expanded.getChildren().removeAll();
                expanded.getChildren().clear();
                File listOfFiles[] = expanded.getValue().listFiles();
                if (listOfFiles != null) {
                    for (File file2 : listOfFiles) {
                        expanded.getChildren().add(new TreeItem<File>(file2));
                    }
                }
            }
            for (TreeItem<File> f : expanded.getChildren()) {
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

    //do wypisywania ścieżek w paskach
    private void getDirectory() {

        files.getSelectionModel().selectedItemProperty().addListener(new ChangeListener() {

            @Override
            public void changed(ObservableValue observable, Object oldValue,
                                Object newValue) {

                TreeItem<File> file = (TreeItem) newValue;
                fileName.setText(file.getValue().toString());
                sizeText.setText(readableFileSize(file.getValue().length()));
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
                if (file != serverFiles.getRoot() && file.getValue() != null) {
                    fileName.setText("/" + getDir(file) + file.getValue().toString());
                    sizeText.setText(readableFileSize(Long.parseLong(file.getSize())));
                    upload.setDisable(true);
                    download.setDisable(false);
                    selectedRemoteFile = file;
                }
            }

        });

    }

    public static String readableFileSize(long size) {
        if(size <= 0) return "0";
        final String[] units = new String[] { "B", "kB", "MB", "GB", "TB" };
        int digitGroups = (int) (Math.log10(size)/Math.log10(1024));
        return new DecimalFormat("#,##0.#").format(size/Math.pow(1024, digitGroups)) + " " + units[digitGroups];
    }

    public String getDir(TreeItemExtended file) {
        String path = "";
        while (file.getParent() != serverFiles.getRoot()) {
            String dir = file.getParent().getValue().toString() + "/";
            path = dir + path;
            file = (TreeItemExtended) file.getParent();
        }
        return path;
    }


    private TreeItem<File> getRootDir() {
        TreeItem<File> root = new TreeItem<File>(new File("Local Computer"));
        Iterable<Path> rootDirectories = FileSystems.getDefault().getRootDirectories();
        for (Path name : rootDirectories) {

            TreeItem<File> treeNode = new TreeItem<File>(new File(name.toString()));

            root.getChildren().add(treeNode);
        }
        root.setExpanded(false);
        return root;
    }
}
