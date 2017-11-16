package sample;

import javafx.fxml.FXML;
import javafx.scene.control.*;

import java.io.*;
import java.lang.reflect.Array;
import java.net.Socket;
import java.net.SocketException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.Arrays;


public class MainScene {
    @FXML
    private TreeView files;
    @FXML
    private Button connect;
    @FXML
    private Label connectionStatus;

    @FXML


    private void initialize() {
        listFiles();
    }


    public MainScene() {

    }

    @FXML
    private void connectWithServer() {
        Socket client = null;
        try {
            client = new Socket("127.0.0.1", 10001);
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

        files.setRoot(getRoot());
    }


    private TreeItem<String> getRoot() {
        TreeItem<String> root = new TreeItem<String>("Root Node");
        root.setExpanded(true);
        root.getChildren().addAll(
                new TreeItem<String>("Item 1"),
                new TreeItem<String>("Item 2"),
                new TreeItem<String>("Item 3"));
        return root;
    }


}
