package fx;

import java.io.File;
import java.io.FilenameFilter;

public class ExplorerServer {
	
	public static void main(String[] args) {
		
		File dir = new File(".");
		
		FilenameFilter filter = new FilenameFilter() {
			
			@Override
			public boolean accept(File dir, String name) {
				return name.matches("...\\.xml");
			}
		};
		
		File[] files = dir.listFiles(filter);
		for (File file : files) {
			System.out.println(file.getName());
		}
	}

}
