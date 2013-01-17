import java.io.*;

public class Logger {
	private PrintWriter pw;
	private long id;

	public Logger() throws IOException {
		id = System.currentTimeMillis() / 1000;
		String fName = "./logs/" + id + ".log";
		File f = new File(fName);

		if (f.exists() && f.isFile()) {
			f.delete();
		}

		this.pw = new PrintWriter(new FileWriter(fName), true);	
	}

	public void log(String message) {
		System.out.println("# Log (" + this.id + "): " + message);
		this.pw.println(message);
	}

	public void close() {
		this.pw.close();
	}
}