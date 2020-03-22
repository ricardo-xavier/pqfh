package br.com.avancoinfo.terminal;

public class Ponto {

	private int y;
	private int x;
	private String id;
	
	public Ponto(int y, int x, String id) {
		this.y = y;
		this.x = x;
		this.id = id;
	}
	public int getY() {
		return y;
	}
	public void setY(int y) {
		this.y = y;
	}
	public int getX() {
		return x;
	}
	public void setX(int x) {
		this.x = x;
	}
	public String getId() {
		return id;
	}
	public void setId(String id) {
		this.id = id;
	}
	
}
