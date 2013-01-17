import java.io.*;
import java.net.*;
import java.util.*;

public class Arena {
	// server variables
	final int TABLE_WIDTH = 8;
	final int ABLE_HEIGHT = 8;
	final int MAX_PLAYERS;
	final int SERVER_PORT;

	//robot variables
	final int DEFAULT_LIFE = 20;
	final int DEFAULT_AMMO = 10;
	final int AMMO_PER_TURN = 2;
	final int ACTIONS_PER_TURN = 3;
	final int TIME_LIMIT = 60; // in secs
	final int RESPONSE_TIMEOUT = 500; // in millisecs

	private List<Robot> robots = new ArrayList<Robot>();
	private ServerSocket ss;
	private Logger log;
	
	public static void main(String args[]) throws Exception {
		Arena server;

		if (args.length != 2) {
			System.out.println("Modo de uso: java Arena [PUERTO] [#_JUGADORES]");
			System.exit(-1);
		}

		while (true) {
			server = new Arena(args);

			if (server.waitForPlayers()) {
				server.start();
				server.close();
			}
		}
	}

	public Arena(String args[]) {
		this.SERVER_PORT = Integer.parseInt(args[0]);
		this.MAX_PLAYERS = Integer.parseInt(args[1]);

		try {
			ss = new ServerSocket(SERVER_PORT);
			ss.setSoTimeout(RESPONSE_TIMEOUT);
		} catch (IOException e) {
			System.out.println("No se pudo iniciar el servidor");
			System.exit(-1);
		}

		try {
			this.log = new Logger();
		} catch (IOException e) {
			System.out.println(e);
			System.exit(-1);
		}
	}

	public void start() {
		long timeStart = System.currentTimeMillis();
		String robotMsg;
		int robotsAlive, opponentsCount;
		boolean gameOver = false;
		byte actions; 

		Collections.shuffle(robots);

		for (int i = 0; i < robots.size(); i++) {
			robots.get(i).write("S ACCEPTED" + this.playerNames());
		}

		while (!gameOver) {
			if (System.currentTimeMillis() - timeStart > TIME_LIMIT * 1000) {
				gameOver = true;

				for (int i = 0; i < robots.size(); i++) {
					if (!robots.get(i).isDead()) {
						robots.get(i).write("S DRAW");
					}
				}

				continue;
			}

			robotsAlive = 0;

			for (int i = 0; i < robots.size(); i++) {
				if (!robots.get(i).isDead()) {
					robotsAlive++;
				}
			}

			if (robotsAlive == 1) {
				for (int i = 0; i < robots.size(); i++) {
					if (robots.get(i).isDead()) {
						robots.get(i).write("S LOSE");
					} else {
						robots.get(i).write("S WIN");
					}
				}

				gameOver = true;

				continue;
			}

			// TURNS
			for (int i = 0; i < robots.size(); i++) {
				if (robots.get(i).isDead()) {
					robots.get(i).write("S URDEAD");

					continue;
				}

				robots.get(i).setAmmo(robots.get(i).getAmmo() + AMMO_PER_TURN);

				if (robots.get(i).getAmmo() > DEFAULT_AMMO) {
					robots.get(i).setAmmo(DEFAULT_AMMO);
				}

				opponentsCount = 0;

				for (int k = 0; k < robots.size(); k++) {
					if (!robots.get(k).isDead() && k != i) {
						opponentsCount++; 
					}
				}

				robots.get(i).write("S URTURN " + robots.get(i).getLife() + " " + robots.get(i).getPosition("x") + " " + robots.get(i).getPosition("y") + " " + robots.get(i).getAmmo() + " " + opponentsCount);

				// wait for robot actions
				actions = 0x0; // 00000001 (0x1): move, 00000010 (0x2):seek, 00000100 (0x4):shoot

				for (int j = 0; j < ACTIONS_PER_TURN; j++) {
					robotMsg = robots.get(i).read();

					if (robotMsg.equals("R FINISH")) {
						break;
					} else {
						String[] parts = robotMsg.trim().split(" ");

						if (parts.length >= 2) {
							if (parts[1].equals("MOVE") && parts.length == 3 && (actions & 0x1) == 0x0) {
								actions &= 0x1;
								this.doMove(robots.get(i), parts);
							} else if (parts[1].equals("SEEK") && parts.length == 3 && (actions & 0x2) == 0x0) {
								actions &= 0x2;
								this.doSeek(robots.get(i), parts);
							} else if  (parts[1].equals("SHOOT") && parts.length == 4 && (actions & 0x4) == 0x0) {
								actions &= 0x4;
								this.doShoot(robots.get(i), parts);
							} else {
								robots.get(i).write("S ERROR");
							}
						} else {
							robots.get(i).write("S ERROR");
						}
					}
				}
			}
		}
	}

	public boolean waitForPlayers() {
		int i = 0, new_x = -1, new_y = -1;
		boolean newPositionFound;
		Socket connectionSocket;
		BufferedReader inFromClient;
		DataOutputStream outToClient;
		String line;
		Random rand = new Random();

		System.out.println("Esperando a jugadores en puerto " + this.SERVER_PORT + "...");

		while (i < this.MAX_PLAYERS) {
			try {
				connectionSocket = this.ss.accept();
				inFromClient = new BufferedReader(
					new InputStreamReader(connectionSocket.getInputStream())
				);

				outToClient = new DataOutputStream(
					connectionSocket.getOutputStream()
				);

				line = inFromClient.readLine();
			} catch (SocketTimeoutException e) {
				continue;
			} catch (IOException e) {
				System.out.println("Error al aceptar cliente");
				continue;
			}
			
			try {
				robots.add(new Robot(line, this.log));
				i++;
			} catch (Error e) {
				System.out.println(e);
				continue;
			} catch (Exception e) {
				System.out.println(e);
				continue;
			}

			robots.get(i-1).setLife(this.DEFAULT_LIFE);
			robots.get(i-1).setAmmo(this.DEFAULT_AMMO);

			newPositionFound = false;

			while (!newPositionFound) {
				new_x = rand.nextInt(8);
				new_y = rand.nextInt(8);

				for (int k = 0; k < robots.size(); k++) {
					if (robots.get(k).getPosition("x") != new_x && robots.get(k).getPosition("y") != new_y) {
						newPositionFound = true;
						break;
					}
				}
			}

			robots.get(i-1).setPosition(new_x, new_y);

			robots.get(i-1).read = inFromClient;
			robots.get(i-1).write = outToClient;

			robots.get(i-1).write("S WAIT");
			System.out.println("Nueva conexion (" + robots.get(i-1) + ")");
		}

		return true;
	}

	public String playerNames() {
		String pn = "";

		for (int i = 0; i < robots.size(); i++) {
			pn += " " + robots.get(i);
		}

		return pn;
	}

	public void doShoot(Robot r, String[] cmd) {
		int ammo = r.getAmmo();
		int power = Integer.parseInt(cmd[3]);
		int effective_dmg = power;
		int new_ammo;
		Random rand = new Random(); // fallo aleatorio

		if (effective_dmg > ammo) {
			effective_dmg = ammo;
		}

		Robot found = this.findRobot(r, Integer.parseInt(cmd[2]));

		if (found != null && rand.nextInt(2) == 1) {
			found.receiveDamage(effective_dmg);
			r.write("S SHOOT HIT " + found.getName() + " " + effective_dmg);
		} else {
			r.write("S SHOOT MISS");
		}

		new_ammo = ammo - effective_dmg;
		new_ammo = new_ammo < 0 ? 0 : new_ammo;
		
		r.setAmmo(new_ammo);
	}

	public void doMove(Robot r, String cmd[]) {
		int direction = Integer.parseInt(cmd[2]);
		int new_x = 0, new_y = 0;

		if (direction == 4 || direction == 6) {
			new_y = r.getPosition("y");
		}

		if (direction == 2 || direction == 8) {
			new_x = r.getPosition("x");
		}

		if (direction == 1 || direction == 2 || direction == 3) {
			new_y = r.getPosition("y") - 1;
		}

		if (direction == 7 || direction == 8 || direction == 9) {
			new_y = r.getPosition("y") + 1;
		}

		if (direction == 1 || direction == 4 || direction == 7) {
			new_x = r.getPosition("x") - 1;
		}

		if (direction == 3 || direction == 6 || direction == 9) {
			new_x = r.getPosition("x") + 1;
		}

		///

		if (new_x < 0 || new_x > 7 || new_y < 0 || new_y > 7) {
			r.write("S MOVE BOUNDARY");
			return;
		} else {
			for (int i = 0; i < robots.size(); i++) {
				if (robots.get(i) == r) {
					continue;
				} else {
					if (robots.get(i).getPosition("x") == new_x && robots.get(i).getPosition("y") == new_y) {
						r.write("S MOVE COLLISION " + robots.get(i));

						return;
					}
				}
			}
		}

		r.setPosition(new_x, new_y);
		r.write("S MOVE OK");
	}

	public void doSeek(Robot r, String cmd[]) {
		Robot found = this.findRobot(r, Integer.parseInt(cmd[2]));
		double distance = 0;

		if (found != null) {
			distance = Math.sqrt(
				Math.pow((double)(r.getPosition("x") - found.getPosition("x")), 2) +
				Math.pow((double)(r.getPosition("y") - found.getPosition("y")), 2)
			);

			r.write("S SEEK RETURNED " + found.getName() + " " + (int)distance);
		} else {
			r.write("S SEEK VOID");
		}
	}

	// para shoot y seek
	public Robot findRobot(Robot r, int direcion) {
		Robot found = null;
		int rx = r.getPosition("x");
		int ry = r.getPosition("y");

		for (int i = 0; i < robots.size(); i++) {
			if (found != null) {
				break;
			}

			if (robots.get(i) == r) {
				continue;
			}

			switch (direcion) {
				case 7:
					if (
						robots.get(i).getPosition("x") < rx &&
						robots.get(i).getPosition("y") > ry &&
						robots.get(i).getPosition("x") + robots.get(i).getPosition("y") == rx + ry
					) {
						found = robots.get(i);
					}
				break;

				case 8:
					if (robots.get(i).getPosition("y") > ry && robots.get(i).getPosition("x") == rx) {
						found = robots.get(i);
					}
				break;

				case 9:
					if (
						robots.get(i).getPosition("x") > rx &&
						robots.get(i).getPosition("y") > ry &&
						Math.abs(robots.get(i).getPosition("x") - robots.get(i).getPosition("y")) == Math.abs(rx - ry)
					) {
						found = robots.get(i);
					}
				break;

				case 6:
					if (robots.get(i).getPosition("x") > rx && robots.get(i).getPosition("y") == ry) {
						found = robots.get(i);
					}
				break;

				case 3:
					if (
						robots.get(i).getPosition("x") > rx &&
						robots.get(i).getPosition("y") < ry &&
						robots.get(i).getPosition("x") + robots.get(i).getPosition("y") == rx + ry
					) {
						found = robots.get(i);
					}
				break;
				
				case 2:
					if (robots.get(i).getPosition("y") < ry && robots.get(i).getPosition("x") == rx) {
						found = robots.get(i);
					}
				break;

				case 1:
					if (
						robots.get(i).getPosition("x") < rx &&
						robots.get(i).getPosition("y") < ry &&
						Math.abs(robots.get(i).getPosition("x") - robots.get(i).getPosition("y")) == Math.abs(rx - ry)
					) {
						found = robots.get(i);
					}
				break;

				case 4:
					if (robots.get(i).getPosition("x") < rx && robots.get(i).getPosition("y") == ry) {
						found = robots.get(i);
					}
				break;
			}
		}

		if (found != null && found.isDead()) {
			return null;
		}

		return found;
	}

	public void finalize() {
		this.close();
	}

	public void close() {
		try {
			this.ss.close();
		} catch (IOException e) {}
		this.log.close();
	}
}
