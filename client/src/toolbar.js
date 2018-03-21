import AppBar from 'material-ui/AppBar';
import Button from 'material-ui/Button';
import { LinearProgress } from 'material-ui/Progress';
import MaterialToolbar from 'material-ui/Toolbar';
import React from 'react';
import Typography from 'material-ui/Typography';


/**
 * The Toolbar Component presents an application toolbar
 * for users to have access to global level controls.
 */
class Toolbar extends React.Component {
  /**
   * Toolbar constuctor.
   * @param {object} props
   */
  constructor(props) {
    super(props);
  }

  /**
   * Renders the component to HTML.
   * @return {HTML}
   */
  render() {
    return (
      <AppBar position='absolute' className={this.props.className}>
        <MaterialToolbar>
          <Typography variant='title' color='inherit'>
            dSpaceX
          </Typography>
          <div style={{
            width: '100%',
            display: 'flex',
            flexDirection: 'row-reverse',
          }}>
            <Button variant='raised' style={{ backgroundColor:'white' }}
              onClick={this.props.onConnectClick}
              disabled={this.props.connectedToServer} >
              Connect
            </Button>
          </div>
        </MaterialToolbar>
        {
          this.props.networkActive ?
            <LinearProgress /> :
            []
        }
      </AppBar>
    );
  }
}

export default Toolbar;
