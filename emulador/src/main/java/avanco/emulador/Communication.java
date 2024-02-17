package avanco.emulador;

import com.jcraft.jsch.ChannelShell;
import com.jcraft.jsch.JSch;
import com.jcraft.jsch.Session;

import java.io.InputStream;
import java.io.OutputStream;
import java.util.Properties;

public class Communication extends Thread {
    private Session session;

    @Override
    public void run() {
        try {
            JSch jsch = new JSch();
            String user = "ricardo"; //TODO
            String password = "Super@3025@Avanco"; //TODO
            String host = "claro.avancoinfo.com.br"; //TODO
            int port = 65509; //TODO
            session = jsch.getSession(user, host, port);
            Properties config = new Properties();
            config.setProperty("StrictHostKeyChecking", "no");
            session.setConfig(config);
            session.setPassword(password);
            session.connect();
            ChannelShell channel = (ChannelShell) session.openChannel("shell");
            InputStream input = channel.getInputStream();
            OutputStream output = channel.getOutputStream();
            channel.connect();

            while (true) {
                if (!channel.isConnected()) {
                    throw new Exception("TODO !channel.isConnected()");
                }
                Thread.sleep(1000);
            }

        } catch (Exception e) {
            e.printStackTrace();
        }

    }

    public void close() {
        session.disconnect();
        System.err.println("disconnected");
    }
}
