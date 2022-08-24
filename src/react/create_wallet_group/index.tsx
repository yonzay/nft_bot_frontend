/* eslint-disable @typescript-eslint/no-explicit-any */
import ReactDOM from 'react-dom';
import { Modal } from '../modal';
import '../../style/style.css';
import '../../style/modal_style.css';

class Create_Wallet_Group_Index {
	public static create_wallet_group = (input: string): void => {
		(window as any).electron.ipcRenderer.message('perform', ['submit_action', 'create_wallet_group', input]);
	};
	public static create = (): void => {
		const Element: Modal = new Modal(undefined, 'create_wallet_group', 'Create Wallet Group', 'modal', 'block', 'json', 'ace-modal', false, `${`{\n	"Name": "",\n	"Blockchain": "",\n	"Private_Keys": [\n		\n	]\n}`}`, this.create_wallet_group);
		ReactDOM.render(<Element.Make/>,
		document.getElementById('root'));
	};
}

Create_Wallet_Group_Index.create();