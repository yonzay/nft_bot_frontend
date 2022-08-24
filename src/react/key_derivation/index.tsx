/* eslint-disable @typescript-eslint/no-explicit-any */
import ReactDOM from 'react-dom';
import { Modal } from '../modal';
import '../../style/style.css';
import '../../style/modal_style.css';

class Key_Derivation_Index {
	public static key_derivation = (input: string): void => {
		(window as any).electron.ipcRenderer.message('perform', ['submit_action', 'key_derivation', input]);
	};
	public static create = (): void => {
		const Element: Modal = new Modal(undefined, 'key_derivation', 'Key Derivation', 'modal', 'block', 'json', 'ace-modal', false, `${`{\n	"Passphrase": "",\n	"Confirm_Passphrase": ""\n}`}`, this.key_derivation);
		ReactDOM.render(<Element.Make/>,
		document.getElementById('root'));
	};
}

Key_Derivation_Index.create();