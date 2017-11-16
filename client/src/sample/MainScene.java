package sample;

import javafx.collections.ObservableList;
import javafx.event.Event;
import javafx.fxml.FXML;
import javafx.scene.control.*;
import javafx.event.EventHandler;
import javafx.scene.control.cell.TextFieldTreeCell;

import java.io.*;
import java.lang.reflect.Array;
import java.net.Socket;
import java.net.SocketException;
import java.nio.charset.StandardCharsets;
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
            client = new Socket("127.0.0.1", 10001);
            //client = new Socket(serverAddress.getText() , Integer.valueOf(portNumber.getText()));
            client = new Socket("150.254.32.67", 10001);
            OutputStream out = client.getOutputStream();
            //out.write("12;3".getBytes());
            System.out.println("wysłano");

            //waits for response
            String path = "test.bin";
            out.write("Request: RETR test.bin".getBytes());

            FileOutputStream stream = new FileOutputStream(path);
            //odpowiedz
            byte buff[] = new byte[100];
            InputStream in = client.getInputStream();
            boolean keepAlive = true;
            while (keepAlive) {
                if (in.available() > 0) {
                    boolean binaryMode = false;
                    BufferedReader reader = new BufferedReader(new InputStreamReader(in));

                    String serverMessage = reader.readLine();
                    System.out.println("bytes received " + in.available());
                    System.out.println("data received " + serverMessage);

                    /*
                    Arrays.fill(buff, (byte) 0);
                    in.read(buff);

                    if (buff[0] == (byte) -1) {
                        System.out.println("Server aborted connection");
                        keepAlive = false;
                        continue;
                    }

                    if ((int)buff[0] == 0)
                    {
                        System.out.println("Saving stream");
                        stream.write(buff);

                    }

                    System.out.println("Received data: " + new String(buff, StandardCharsets.UTF_8));

*/
                }
            }

            stream.close();
            client.close();

        } catch (IOException e) {
            System.out.println("Brak połączenia");
        }


        try {
            listDirectoryFiles("test");
        } catch (RuntimeException e) {
            System.out.println(e.getMessage());
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
