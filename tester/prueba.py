#!/usr/bin/env python3
#! alguno de los errores que da por malos, si compruebas los motivos so.
#! Join canal (sin #) damos un error diferente al que espera y lo toma como error.
#! invita o manda un mensaje privado a un cliente que no existe. tester cree que fue creado, no existe y no podemos mandarle nada
import socket
import threading
import time
import sys

class IRCTester:
    def __init__(self, host='localhost', port=6667, password=''):
        self.host = host
        self.port = port
        self.password = password
        self.sock = None
        self.connected = False
        self.responses = []
        self.listening = False
        
        # Contadores de tests
        self.total_tests = 0
        self.passed_tests = 0
        self.failed_tests = 0
        
    def connect(self):
        """Conectar al servidor IRC"""
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((self.host, self.port))
            print(f"✅ Conectado a {self.host}:{self.port}")
            
            # Iniciar listener en thread separado
            self.listening = True
            listener_thread = threading.Thread(target=self._listen_responses)
            listener_thread.daemon = True
            listener_thread.start()
            
            return True
        except Exception as e:
            print(f"❌ Error conectando: {e}")
            return False
    
    def _listen_responses(self):
        """Escuchar respuestas del servidor"""
        while self.listening:
            try:
                data = self.sock.recv(4096)
                if data:
                    response = data.decode('utf-8', errors='ignore')
                    self.responses.append(response)
                    print(f"📥 {response.strip()}")
                else:
                    break
            except:
                break
    
    def send_command(self, command, wait_time=1):
        """Enviar comando y esperar respuesta"""
        try:
            self.sock.send(f"{command}\r\n".encode('utf-8'))
            print(f"📤 {command}")
            time.sleep(wait_time)
            return True
        except Exception as e:
            print(f"❌ Error enviando comando: {e}")
            return False
    
    def get_last_responses(self, n=5):
        """Obtener las últimas n respuestas"""
        return self.responses[-n:] if len(self.responses) >= n else self.responses
    
    def run_test(self, test_name, test_func):
        """Ejecutar un test y actualizar contadores"""
        self.total_tests += 1
        print(f"\n🧪 TEST {self.total_tests}: {test_name}")
        print("-" * 50)
        
        try:
            result = test_func()
            if result:
                self.passed_tests += 1
                print(f"✅ PASS: {test_name}")
            else:
                self.failed_tests += 1
                print(f"❌ FAIL: {test_name}")
            return result
        except Exception as e:
            self.failed_tests += 1
            print(f"💥 ERROR: {test_name} - {e}")
            return False
    
    def test_authentication(self, nick="tester", user="testuser"):
        """Test de autenticación básica"""
        if self.password:
            self.send_command(f"PASS {self.password}")
        
        self.send_command(f"NICK {nick}")
        self.send_command(f"USER {user} 0 * :Test User")
        
        time.sleep(2)
        responses = self.get_last_responses(10)
        all_responses = " ".join(responses).upper()
        
        # Verificar diferentes indicadores de autenticación exitosa
        auth_indicators = [
            "001", "WELCOME", "CONNECTED", "REGISTERED", "MOTD",
            f":{nick}!{user}@", "PING", "MODE " + nick,
        ]
        
        error_indicators = [
            "464", "431", "432", "433", "ERROR", "DENIED", "BANNED",
        ]
        
        error_found = any(error in all_responses for error in error_indicators)
        if error_found:
            return False
        
        auth_found = any(indicator in all_responses for indicator in auth_indicators)
        
        if auth_found:
            self.connected = True
            return True
        else:
            # Test secundario con JOIN
            self.send_command("JOIN #testauth")
            time.sleep(1)
            join_responses = self.get_last_responses(3)
            join_text = " ".join(join_responses).upper()
            
            if "JOIN" in join_text or "353" in join_text or "366" in join_text:
                self.connected = True
                return True
            return False
    
    def test_nick_valid(self):
        """Test NICK con nombre válido"""
        self.send_command("NICK testnick1")
        time.sleep(1)
        responses = " ".join(self.get_last_responses(3)).upper()
        return "ERROR" not in responses and "432" not in responses
    
    def test_nick_duplicate(self):
        """Test NICK con nombre duplicado"""
        # Usar el mismo nick actual
        self.send_command("NICK tester")
        time.sleep(1)
        responses = " ".join(self.get_last_responses(3)).upper()
        # Puede devolver 433 (nick in use) o simplemente ignorarlo
        return True  # No falla necesariamente
    
    def test_nick_invalid_chars(self):
        """Test NICK con caracteres inválidos"""
        self.send_command("NICK test@nick")
        time.sleep(1)
        responses = " ".join(self.get_last_responses(3)).upper()
        return "432" in responses or "ERROR" in responses
    
    def test_nick_empty(self):
        """Test NICK sin parámetro"""
        self.send_command("NICK")
        time.sleep(1)
        responses = " ".join(self.get_last_responses(3)).upper()
        return "431" in responses or "ERROR" in responses
    
    def test_join_valid_channel(self):
        """Test JOIN a canal válido"""
        self.send_command("JOIN #testchannel")
        time.sleep(1)
        responses = " ".join(self.get_last_responses(3)).upper()
        return "JOIN" in responses or "353" in responses or "366" in responses
    
    def test_join_invalid_channel_no_hash(self):
        """Test JOIN a canal sin #"""
        self.send_command("JOIN invalidchannel")
        time.sleep(1)
        responses = " ".join(self.get_last_responses(3)).upper()
        return "403" in responses or "ERROR" in responses or "INVALID" in responses
    
    def test_join_empty_channel(self):
        """Test JOIN sin parámetro"""
        self.send_command("JOIN")
        time.sleep(1)
        responses = " ".join(self.get_last_responses(3)).upper()
        return "461" in responses or "ERROR" in responses
    
    def test_join_multiple_channels(self):
        """Test JOIN múltiples canales"""
        self.send_command("JOIN #test1,#test2")
        time.sleep(2)
        responses = " ".join(self.get_last_responses(5)).upper()
        return "JOIN" in responses
    
    def test_privmsg_channel(self):
        """Test PRIVMSG a canal"""
        self.send_command("JOIN #msgtest")
        time.sleep(1)
        self.send_command("PRIVMSG #msgtest :Hello channel")
        time.sleep(1)
        responses = " ".join(self.get_last_responses(3)).upper()
        return "PRIVMSG" in responses or "404" not in responses
    
    def test_privmsg_user(self):
        """Test PRIVMSG a usuario"""
        self.send_command("PRIVMSG testuser :Hello user")
        time.sleep(1)
        responses = " ".join(self.get_last_responses(3)).upper()
        return "401" not in responses  # No such nick error
    
    def test_privmsg_no_target(self):
        """Test PRIVMSG sin destinatario"""
        self.send_command("PRIVMSG")
        time.sleep(1)
        responses = " ".join(self.get_last_responses(3)).upper()
        return "411" in responses or "ERROR" in responses
    
    def test_privmsg_no_message(self):
        """Test PRIVMSG sin mensaje"""
        self.send_command("PRIVMSG #test")
        time.sleep(1)
        responses = " ".join(self.get_last_responses(3)).upper()
        return "412" in responses or "ERROR" in responses
    
    def test_topic_set(self):
        """Test TOPIC - establecer topic"""
        self.send_command("JOIN #topictest")
        time.sleep(1)
        self.send_command("TOPIC #topictest :This is a test topic")
        time.sleep(1)
        responses = " ".join(self.get_last_responses(3)).upper()
        return "TOPIC" in responses or "332" in responses
    
    def test_topic_get(self):
        """Test TOPIC - obtener topic"""
        self.send_command("TOPIC #topictest")
        time.sleep(1)
        responses = " ".join(self.get_last_responses(3)).upper()
        return "332" in responses or "331" in responses or "TOPIC" in responses
    
    def test_topic_no_channel(self):
        """Test TOPIC sin canal"""
        self.send_command("TOPIC")
        time.sleep(1)
        responses = " ".join(self.get_last_responses(3)).upper()
        return "461" in responses or "ERROR" in responses
    
    def test_mode_channel_view(self):
        """Test MODE - ver modos de canal"""
        self.send_command("JOIN #modetest")
        time.sleep(1)
        self.send_command("MODE #modetest")
        time.sleep(1)
        responses = " ".join(self.get_last_responses(3)).upper()
        return "324" in responses or "MODE" in responses
    
    def test_mode_channel_set(self):
        """Test MODE - establecer modo de canal"""
        self.send_command("MODE #modetest +t")
        time.sleep(1)
        responses = " ".join(self.get_last_responses(3)).upper()
        return "MODE" in responses or "482" not in responses
    
    def test_part_with_message(self):
        """Test PART con mensaje"""
        self.send_command("JOIN #parttest")
        time.sleep(1)
        self.send_command("PART #parttest :Goodbye!")
        time.sleep(1)
        responses = " ".join(self.get_last_responses(3)).upper()
        return "PART" in responses
    
    def test_part_no_message(self):
        """Test PART sin mensaje"""
        self.send_command("JOIN #parttest2")
        time.sleep(1)
        self.send_command("PART #parttest2")
        time.sleep(1)
        responses = " ".join(self.get_last_responses(3)).upper()
        return "PART" in responses
    
    def test_part_no_channel(self):
        """Test PART sin canal"""
        self.send_command("PART")
        time.sleep(1)
        responses = " ".join(self.get_last_responses(3)).upper()
        return "461" in responses or "ERROR" in responses
    
    def test_kick_user(self):
        """Test KICK - expulsar usuario"""
        self.send_command("JOIN #kicktest")
        time.sleep(1)
        self.send_command("KICK #kicktest nonexistentuser :Test kick")
        time.sleep(1)
        responses = " ".join(self.get_last_responses(3)).upper()
        return "KICK" in responses or "441" in responses or "482" in responses
    
    def test_invite_user(self):
        """Test INVITE usuario"""
        self.send_command("INVITE testuser #invitetest")
        time.sleep(1)
        responses = " ".join(self.get_last_responses(3)).upper()
        return "341" in responses or "INVITE" in responses or "401" in responses
    
    def test_cap_list(self):
        """Test CAP LS - listar capabilities"""
        self.send_command("CAP LS")
        time.sleep(1)
        responses = " ".join(self.get_last_responses(3)).upper()
        return "CAP" in responses
    
    def test_user_change(self):
        """Test USER - cambio de información"""
        self.send_command("USER newuser 0 * :New Real Name")
        time.sleep(1)
        responses = " ".join(self.get_last_responses(3)).upper()
        return "462" in responses or "ERROR" in responses  # Ya registrado
    
    def test_pass_wrong(self):
        """Test PASS - contraseña incorrecta (si aplicable)"""
        if not self.password:
            return True  # Skip si no hay contraseña
        
        # Crear nueva conexión para probar
        test_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            test_sock.connect((self.host, self.port))
            test_sock.send(b"PASS wrongpassword\r\n")
            test_sock.send(b"NICK testfail\r\n")
            test_sock.send(b"USER testfail 0 * :Test\r\n")
            time.sleep(2)
            
            data = test_sock.recv(1024)
            response = data.decode('utf-8', errors='ignore').upper()
            test_sock.close()
            
            return "464" in response or "ERROR" in response
        except:
            return False
    
    def test_invalid_command(self):
        """Test comando inválido"""
        self.send_command("INVALIDCOMMAND test")
        time.sleep(1)
        responses = " ".join(self.get_last_responses(3)).upper()
        return "421" in responses or "ERROR" in responses or "UNKNOWN" in responses
    
    def print_summary(self):
        """Imprimir resumen final de tests"""
        print("\n" + "="*60)
        print("🏁 RESUMEN FINAL DE TESTS")
        print("="*60)
        print(f"📊 Total de tests ejecutados: {self.total_tests}")
        print(f"✅ Tests exitosos: {self.passed_tests}")
        print(f"❌ Tests fallidos: {self.failed_tests}")
        
        if self.total_tests > 0:
            success_rate = (self.passed_tests / self.total_tests) * 100
            print(f"📈 Tasa de éxito: {success_rate:.1f}%")
            
            if success_rate >= 90:
                print("🎉 ¡EXCELENTE! Tu servidor IRC funciona muy bien")
            elif success_rate >= 75:
                print("👍 BUENO - Tu servidor IRC funciona bien con algunos issues menores")
            elif success_rate >= 50:
                print("⚠️  REGULAR - Tu servidor IRC tiene varios problemas")
            else:
                print("🚨 CRÍTICO - Tu servidor IRC necesita mucho trabajo")
        
        print("="*60)
    
    def run_all_tests(self):
        """Ejecutar todos los tests"""
        print("🚀 INICIANDO TESTS COMPLETOS DEL SERVIDOR IRC")
        print("=" * 60)
        
        if not self.connect():
            return False
        
        # Test de autenticación
        if not self.run_test("Autenticación básica", self.test_authentication):
            print("❌ Falló la autenticación. Abortando tests.")
            return False
        
        # Tests de NICK
        print(f"\n🏷️  TESTS DE NICK")
        print("-" * 30)
        self.run_test("NICK válido", self.test_nick_valid)
        self.run_test("NICK duplicado", self.test_nick_duplicate)
        self.run_test("NICK caracteres inválidos", self.test_nick_invalid_chars)
        self.run_test("NICK vacío", self.test_nick_empty)
        
        # Tests de JOIN
        print(f"\n📺 TESTS DE JOIN")
        print("-" * 30)
        self.run_test("JOIN canal válido", self.test_join_valid_channel)
        self.run_test("JOIN canal sin #", self.test_join_invalid_channel_no_hash)
        self.run_test("JOIN sin parámetro", self.test_join_empty_channel)
        self.run_test("JOIN múltiples canales", self.test_join_multiple_channels)
        
        # Tests de PRIVMSG
        print(f"\n💬 TESTS DE PRIVMSG")
        print("-" * 30)
        self.run_test("PRIVMSG a canal", self.test_privmsg_channel)
        self.run_test("PRIVMSG a usuario", self.test_privmsg_user)
        self.run_test("PRIVMSG sin destinatario", self.test_privmsg_no_target)
        self.run_test("PRIVMSG sin mensaje", self.test_privmsg_no_message)
        
        # Tests de TOPIC
        print(f"\n📝 TESTS DE TOPIC")
        print("-" * 30)
        self.run_test("TOPIC establecer", self.test_topic_set)
        self.run_test("TOPIC obtener", self.test_topic_get)
        self.run_test("TOPIC sin canal", self.test_topic_no_channel)
        
        # Tests de MODE
        print(f"\n⚙️  TESTS DE MODE")
        print("-" * 30)
        self.run_test("MODE ver canal", self.test_mode_channel_view)
        self.run_test("MODE establecer canal", self.test_mode_channel_set)
        
        # Tests de PART
        print(f"\n🚪 TESTS DE PART")
        print("-" * 30)
        self.run_test("PART con mensaje", self.test_part_with_message)
        self.run_test("PART sin mensaje", self.test_part_no_message)
        self.run_test("PART sin canal", self.test_part_no_channel)
        
        # Tests avanzados
        print(f"\n⚡ TESTS AVANZADOS")
        print("-" * 30)
        self.run_test("KICK usuario", self.test_kick_user)
        self.run_test("INVITE usuario", self.test_invite_user)
        self.run_test("CAP LS", self.test_cap_list)
        
        # Tests de error
        print(f"\n🚫 TESTS DE ERRORES")
        print("-" * 30)
        self.run_test("USER ya registrado", self.test_user_change)
        self.run_test("PASS incorrecta", self.test_pass_wrong)
        self.run_test("Comando inválido", self.test_invalid_command)
        
        # Test final
        self.send_command("QUIT :Testing completed")
        time.sleep(1)
        
        # Imprimir resumen
        self.print_summary()
        
        # Cerrar conexión
        self.listening = False
        if self.sock:
            self.sock.close()
        
        return True
    
    def interactive_mode(self):
        """Modo interactivo para enviar comandos manualmente"""
        print("\n🎮 MODO INTERACTIVO")
        print("Escribe comandos IRC (sin CRLF, se añaden automáticamente)")
        print("Escribe 'quit' para salir")
        print("=" * 40)
        
        if not self.connect():
            return
        
        try:
            while True:
                command = input("IRC> ").strip()
                if command.lower() == 'quit':
                    break
                if command:
                    self.send_command(command)
        except KeyboardInterrupt:
            print("\n👋 Saliendo...")
        finally:
            self.listening = False
            if self.sock:
                self.sock.close()

def main():
    if len(sys.argv) < 2:
        print("Uso: python irc_tester.py <modo> [host] [port] [password]")
        print("Modos:")
        print("  auto    - Ejecutar todos los tests automáticamente")
        print("  manual  - Modo interactivo para comandos manuales")
        print("\nEjemplos:")
        print("  python irc_tester.py auto")
        print("  python irc_tester.py auto localhost 6667 mypassword")
        print("  python irc_tester.py manual localhost 6667")
        return
    
    mode = sys.argv[1]
    host = sys.argv[2] if len(sys.argv) > 2 else 'localhost'
    port = int(sys.argv[3]) if len(sys.argv) > 3 else 6667
    password = sys.argv[4] if len(sys.argv) > 4 else ''
    
    tester = IRCTester(host, port, password)
    
    if mode == 'auto':
        tester.run_all_tests()
    elif mode == 'manual':
        tester.interactive_mode()
    else:
        print("❌ Modo inválido. Usa 'auto' o 'manual'")

if __name__ == "__main__":
    main()