/* eslint-disable @typescript-eslint/no-explicit-any */
import ReactDOM from 'react-dom';
import { Modal } from '../modal';
import '../../style/style.css';
import '../../style/modal_style.css';

class Authentication_Index {
	public static authentication = (input: string): void => {
		(window as any).electron.ipcRenderer.message('perform', ['submit_action', 'authentication', input]);
	};
	public static create = (): void => {
		const Element: Modal = new Modal(undefined, 'authentication', 'Authentication', 'modal', 'block', 'json', 'ace-modal', false, `${`{\n	"Hyper_License_Key": ""\n}`}`, this.authentication);
		ReactDOM.render(<Element.Make/>,
		document.getElementById('root'));
	};
}

Authentication_Index.create();