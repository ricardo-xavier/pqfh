package fx;

import com.jfoenix.controls.datamodels.treetable.RecursiveTreeObject;

import javafx.beans.property.IntegerProperty;
import javafx.beans.property.SimpleIntegerProperty;
import javafx.beans.property.SimpleStringProperty;
import javafx.beans.property.StringProperty;

public class FileNode extends RecursiveTreeObject<FileNode> {
	
	private StringProperty name;
	private StringProperty date;
	private IntegerProperty size;
	
	public FileNode(String name, String date, Integer size) {
		this.name = new SimpleStringProperty(name);
		this.date = new SimpleStringProperty(date);
		this.size = new SimpleIntegerProperty(size);
	}
	
	public StringProperty getName() {
		return name;
	}
	public void setName(StringProperty name) {
		this.name = name;
	}
	public StringProperty getDate() {
		return date;
	}
	public void setDate(StringProperty date) {
		this.date = date;
	}
	public IntegerProperty getSize() {
		return size;
	}
	public void setSize(IntegerProperty size) {
		this.size = size;
	}

}
