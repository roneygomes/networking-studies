use std::env;
use std::io::Read;
use std::net::TcpStream;

const PORT: u16 = 3490;

fn main() {
    let hostname = env::args().nth(1);

    if hostname.is_none() {
        eprintln!("usage: client hostname");
        return;
    }

    match TcpStream::connect((hostname.unwrap().as_str(), PORT)) {
        Ok(mut stream) => {
            if let Ok(addr) = stream.peer_addr() {
                println!("connecting to host {}", addr.ip());
            }

            read_socket(&mut stream);
        }
        Err(e) => {
            eprintln!("failed to connect to socket; cause: {e}");
        }
    }
}

fn read_socket(stream: &mut TcpStream) {
    let mut buffer = [0; 128];

    match stream.read(&mut buffer) {
        Ok(_) => {
            match String::from_utf8(buffer.into()) {
                Ok(s) => println!("received: {s}"),
                Err(_) => eprintln!("server didn't send a valid utf-8 string")
            }
        }
        Err(e) => eprintln!("failed to read from socket; cause; {e}")
    }
}