/* eslint-disable @typescript-eslint/no-explicit-any */
import ReactDOM from 'react-dom';
import { Modal } from '../modal';
import '../../style/style.css';
import '../../style/modal_style.css';

class Generate_Wallet_Index {
	public static generate_wallet = (input: string): void => {
		(window as any).electron.ipcRenderer.message('perform', ['submit_action', 'generate_wallet', input]);
	};
	public static create = (): void => {
		const Element: Modal = new Modal(undefined, 'generate_wallet', 'Generate Wallet', 'modal', 'block', 'json', 'ace-modal', false, `${`{\n	"Blockchain": ""\n}`}`, this.generate_wallet);
		ReactDOM.render(<Element.Make/>,
		document.getElementById('root'));
	};
}

Generate_Wallet_Index.create();