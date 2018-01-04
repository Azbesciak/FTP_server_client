package sample;

import javafx.scene.control.TreeItem;

public class TreeItemExtended<T> extends TreeItem {

    private String size;
    public TreeItemExtended(String data)
    {
        try {
            String[] fileData = data.split("/");
            if (fileData[0].charAt(0) == 'P') {
                this.setValue(fileData[0].substring(1));
                this.size = fileData[1];
            }
            if (fileData[0].charAt(0) == 'F') {
                this.setValue(fileData[0].substring(1));
                this.size = fileData[1];
                this.getChildren().add(new TreeItemExtended<>(" / "));
            }
        }
        catch(Exception e)
        {
            System.out.println(e.toString());
        }
    }

    public String getSize() {
        return size;
    }

    public void setSize(String size) {
        this.size = size;
    }
}
