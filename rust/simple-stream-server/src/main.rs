use std::io;
use std::io::Write;
use std::net::{Ipv4Addr, Ipv6Addr, SocketAddr, TcpListener, TcpStream};

use libc;

static HELLO_WORLD: &str = "Hello World!";
const PORT: u16 = 3490;

fn main() {
    let addresses = [
        SocketAddr::from((Ipv6Addr::LOCALHOST, PORT)),
        SocketAddr::from((Ipv4Addr::LOCALHOST, PORT))
    ];

    match TcpListener::bind(&addresses[..]) {
        Ok(listener) => {
            println!("waiting for connections...");

            for stream in listener.incoming() {
                handle_connection(stream);
            }
        }
        Err(e) => {
            eprintln!("failed to bind socket; cause: {e}");
        }
    }
}

fn handle_connection(stream_result: io::Result<TcpStream>) {
    match stream_result {
        Ok(mut stream) =>  {
            if let Ok(addr) = stream.peer_addr() {
                println!("received connection from {}", addr.ip());
            }

            let pid = unsafe { libc::fork() };

            if pid == -1 {
                eprintln!("failed to handle connection; cause: could not fork process");
                return;
            }

            if pid == 0 {
                if let Err(e) = stream.write(HELLO_WORLD.as_bytes()) {
                    eprintln!("failed to send message to remote host; cause {e}");
                }
            }
        }
        Err(e) => eprintln!("failed to handle connection; cause: {e}")
    }
}