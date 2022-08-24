/* eslint-disable @typescript-eslint/no-explicit-any */
import ReactDOM from 'react-dom';
import { Modal } from '../modal';
import '../../style/style.css';
import '../../style/modal_style.css';

class Decrypt_Index {
	public static decrypt = (input: string): void => {
		(window as any).electron.ipcRenderer.message('perform', ['submit_action', 'decrypt', input]);
	};
	public static create = (): void => {
		const Element: Modal = new Modal(undefined, 'decrypt', 'Decrypt', 'modal', 'block', 'json', 'ace-modal', false, `${`{\n	"Passphrase": ""\n}`}`, this.decrypt);
		ReactDOM.render(<Element.Make/>,
		document.getElementById('root'));
	};
}

Decrypt_Index.create();