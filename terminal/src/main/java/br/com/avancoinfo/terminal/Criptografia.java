package br.com.avancoinfo.terminal;

import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.security.spec.InvalidKeySpecException;

import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.DESKeySpec;

import org.apache.commons.codec.DecoderException;
import org.apache.commons.codec.binary.Hex;

public class Criptografia {
	
	private byte[] keyBytes = "avancoin".getBytes();
	
	public String criptografa(String senha) throws NoSuchAlgorithmException, NoSuchPaddingException, InvalidKeyException, IllegalBlockSizeException, BadPaddingException, InvalidKeySpecException {
	
		SecretKeyFactory factory = SecretKeyFactory.getInstance("DES");
		SecretKey chaveDES = factory.generateSecret(new DESKeySpec(keyBytes));
		Cipher cifraDES = Cipher.getInstance("DES/ECB/PKCS5Padding");
		cifraDES.init(Cipher.ENCRYPT_MODE, chaveDES);
		byte[] textoPuro = senha.getBytes();
		byte[] textoEncriptado = cifraDES.doFinal(textoPuro);
		String hexString = Hex.encodeHexString(textoEncriptado);
		return hexString;
	}
	
	public String descriptografa(String senha) throws NoSuchAlgorithmException, NoSuchPaddingException, InvalidKeyException, IllegalBlockSizeException, BadPaddingException, InvalidKeySpecException, DecoderException {
		
		SecretKeyFactory factory = SecretKeyFactory.getInstance("DES");
		SecretKey chaveDES = factory.generateSecret(new DESKeySpec(keyBytes));
		Cipher cifraDES = Cipher.getInstance("DES/ECB/PKCS5Padding");
		cifraDES.init(Cipher.DECRYPT_MODE, chaveDES);
		byte[] aux = Hex.decodeHex(senha.toCharArray());
		byte[] textoDecriptografado = cifraDES.doFinal(aux);
		return new String(textoDecriptografado);
	}
	
	public static void main(String[] args) throws InvalidKeyException, NoSuchAlgorithmException, NoSuchPaddingException, IllegalBlockSizeException, BadPaddingException, InvalidKeySpecException, DecoderException {
		String senha = "Bigu";
		Criptografia cript = new Criptografia();
		String aux = cript.criptografa(senha);
		System.out.println(aux);
		senha = cript.descriptografa(aux);
		System.out.println(senha);
	}

}
