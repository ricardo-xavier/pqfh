package br.com.avancoinfo.pendencias;

import com.jfoenix.controls.datamodels.treetable.RecursiveTreeObject;

import javafx.beans.property.SimpleStringProperty;
import javafx.beans.property.StringProperty;

public class Pendencia extends RecursiveTreeObject<Pendencia> {
	
	private StringProperty chave;
    private StringProperty data;
    private StringProperty cupom;
    private StringProperty nota;
    private StringProperty serie;
    private StringProperty tipo;
    private StringProperty situacao;
    private StringProperty status;
    
    public Pendencia(String chave, String data, String cupom, String nota, String serie, String tipo, String situacao, String status) {
    	this.setChave(new SimpleStringProperty(chave));
        this.setData(new SimpleStringProperty(data));
        this.setCupom(new SimpleStringProperty(cupom));
        this.setNota(new SimpleStringProperty(nota));
        this.setSerie(new SimpleStringProperty(serie));
        this.setTipo(new SimpleStringProperty(tipo));
        this.setSituacao(new SimpleStringProperty(situacao));
        this.setStatus(new SimpleStringProperty(status));
    }

	public StringProperty getData() {
		return data;
	}

	public void setData(StringProperty data) {
		this.data = data;
	}

	public StringProperty getCupom() {
		return cupom;
	}

	public void setCupom(StringProperty cupom) {
		this.cupom = cupom;
	}

	public StringProperty getNota() {
		return nota;
	}

	public void setNota(StringProperty nota) {
		this.nota = nota;
	}

	public StringProperty getSerie() {
		return serie;
	}

	public void setSerie(StringProperty serie) {
		this.serie = serie;
	}

	public StringProperty getTipo() {
		return tipo;
	}

	public void setTipo(StringProperty tipo) {
		this.tipo = tipo;
	}

	public StringProperty getChave() {
		return chave;
	}

	public void setChave(StringProperty chave) {
		this.chave = chave;
	}

	public StringProperty getSituacao() {
		return situacao;
	}

	public void setSituacao(StringProperty situacao) {
		this.situacao = situacao;
	}

	public StringProperty getStatus() {
		return status;
	}

	public void setStatus(StringProperty status) {
		this.status = status;
	}

}
