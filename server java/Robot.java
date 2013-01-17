import java.io.*;
import java.net.*;
import java.util.*;

public class Robot {
	private String name;
	private int life;
	private int ammo;
	private int position[] = {-1, -1};
	private boolean isDead = false;
	private Logger logger;
	public BufferedReader read;
	public DataOutputStream write;

	public Robot(String connectMsg, Logger logger) throws Error {
		String parts[] = connectMsg.split(" ");

		if (parts[0].equals("R") && parts[1].equals("NAME")) {
			this.setName(parts[2]);
		} else {
			throw new Error("Mensaje de iniciacion invalido: " + connectMsg);
		}

		this.logger = logger;
	}

	public void setName(String name) {
		this.name = name;
	}

	public void setLife(int life) {
		this.life = life;
	}

	public void setAmmo(int ammo) {
		this.ammo = ammo;
	}

	public void setPosition(int x, int y) {
		this.position[0] = x;
		this.position[1] = y;
	}

	public String getName() {
		return this.name;
	}

	public int getLife() {
		return this.life;
	}

	public int getAmmo() {
		return this.ammo;
	}

	public int[] getPosition() {
		return this.position;
	}

	public void receiveDamage(int dmg) {
		this.life -= dmg;

		if (this.life <= 0) {
			this.isDead = true;
		}
	}

	public boolean isDead() {
		return this.isDead;
	}
	
	public int getPosition(String coord) {
		int p[] = this.getPosition();

		if (coord.equalsIgnoreCase("y")) {
			return p[1];
		} else {
			return p[0];
		}
	}

	public String toString() {
		return this.getName();
	}

	public String read() {
		String line = "R FINISH";

		try {
			line = this.read.readLine();
		} catch (SocketTimeoutException e) {
			line = "R FINISH";
		} catch (IOException e) {
			// error al leer, cliente desconectado
			line = "R FINISH";
			this.isDead = true;
		} finally {
			if (line == null) {
				line = "R FINISH";
			}

			this.logger.log("Server <- [" + this + "] {" + line + "}");

			return line;
		}
	}

	public void write(String message) {
		try {
			this.logger.log("Server -> [" + this + "] {" + message + "}");
			this.write.writeBytes(message + "\n");
		} catch (IOException e) {
			// error al escribir, cliente desconectado
			this.isDead = true;
		}
	}
}