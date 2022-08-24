import ReactDOM from 'react-dom';
import { Main } from '../main';
import '../../style/style.css';

class Index {
	public static create = (): void => {
		ReactDOM.render(<Main.Make/>,
		document.getElementById('root'));
	};
}

Index.create();