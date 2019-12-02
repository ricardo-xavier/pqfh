package br.com.avancoinfo.pendencias;

import com.jfoenix.controls.datamodels.treetable.RecursiveTreeObject;

import javafx.beans.property.SimpleStringProperty;
import javafx.beans.property.StringProperty;

public class Log extends RecursiveTreeObject<Log> {
	
    private StringProperty data;
    private StringProperty descricao;
    
    public Log(String data, String descricao) {
        this.setData(new SimpleStringProperty(data));
        this.setDescricao(new SimpleStringProperty(descricao));
    }

	public StringProperty getData() {
		return data;
	}

	public void setData(StringProperty data) {
		this.data = data;
	}

	public StringProperty getDescricao() {
		return descricao;
	}

	public void setDescricao(StringProperty descricao) {
		this.descricao = descricao;
	}

}
