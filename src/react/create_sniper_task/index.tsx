/* eslint-disable @typescript-eslint/no-explicit-any */
import ReactDOM from 'react-dom';
import { Modal } from '../modal';
import '../../style/style.css';
import '../../style/modal_style.css';

class Create_Sniper_Task {
	public static create_sniper_task = (input: string): void => {
		(window as any).electron.ipcRenderer.message('perform', ['submit_action', 'create_sniper_task', input]);
	};
	public static create = (): void => {
		const Element: Modal = new Modal(undefined, 'create_sniper_task', 'Create Sniper Task', 'modal', 'block', 'json', 'ace-modal', false, `${`{\n	"Blockchain": "",\n	"Marketplace": "",\n	"Marketplace_API_Key": "",\n	"Collection": "",\n	"Ethereum": {\n		"Use_Flashbots": "",\n		"ERC_20_Token_Address": "",\n		"Gas_Price": "",\n		"Gas_Limit": "",\n		"Priority_Fee": "",\n		"Nonce": ""\n	},\n	"Wallet_Private_Key": "",\n	"Minimum_Value": "",\n	"Maximum_Value": "",\n	"Delay": ""\n}`}`, this.create_sniper_task);
		ReactDOM.render(<Element.Make/>,
		document.getElementById('root'));
	};
}

Create_Sniper_Task.create();