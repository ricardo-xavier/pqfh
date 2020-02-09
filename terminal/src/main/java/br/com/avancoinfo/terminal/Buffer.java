package br.com.avancoinfo.terminal;

public class Buffer {
	
	private int tam;
	private byte[] dados;
	
	public Buffer(int tam, byte[] dados) {
		super();
		this.tam = tam;
		this.dados = dados;
	}
	public int getTam() {
		return tam;
	}
	public void setTam(int tam) {
		this.tam = tam;
	}
	public byte[] getDados() {
		return dados;
	}
	public void setDados(byte[] dados) {
		this.dados = dados;
	}

}
