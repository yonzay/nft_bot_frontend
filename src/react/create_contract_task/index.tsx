/* eslint-disable @typescript-eslint/no-explicit-any */
import ReactDOM from 'react-dom';
import { Modal } from '../modal';
import '../../style/style.css';
import '../../style/modal_style.css';

class Create_Contract_Task_Index {
	public static create_contract_task = (input: string): void => {
		(window as any).electron.ipcRenderer.message('perform', ['submit_action', 'create_contract_task', input]);
	};
	public static create = (): void => {
		const Element: Modal = new Modal(undefined, 'create_contract_task', 'Create Contract Task', 'modal', 'block', 'json', 'ace-modal', false, `${`{\n	"Blockchain": "",\n	"Wallet_Group": "",\n	"Ethereum": {\n		"Contract_Address": "",\n		"Function": "",\n		"Call_Data": [\n			{\n				"type": "",\n				"value": ""\n			}\n		],\n		"Poll": {\n			"Type": "",\n			"Non_Flip": {\n				"Variable_Name": "",\n				"Value": ""\n			},\n			"Flip": {\n				"Owner_Address": "",\n				"Function": ""\n			},\n			"Timeout": ""\n		},\n		"Value": "",\n		"Gas_Price": "",\n		"Gas_Limit": "",\n		"Priority_Fee": ""\n	},\n	"Solana": {\n		"Mode": "CMV1",\n		"Candy_Machine_V1": {\n			"Id": "",\n			"Amount": ""\n		},\n		"Candy_Machine_V2": {\n			"Id": "",\n			"Amount": ""\n		}\n	}\n}`}`, this.create_contract_task);
		ReactDOM.render(<Element.Make/>,
		document.getElementById('root'));
	};
}

Create_Contract_Task_Index.create();