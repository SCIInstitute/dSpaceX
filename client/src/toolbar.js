import AppBar from '@material-ui/core/AppBar';
import Button from '@material-ui/core/Button';
import Icon from '@mdi/react';
import LinearProgress from '@material-ui/core/LinearProgress';
import MaterialToolbar from '@material-ui/core/Toolbar';
import PropTypes from 'prop-types';
import React from 'react';
import Typography from '@material-ui/core/Typography';
import { mdiFilter } from '@mdi/js';
import { withStyles } from '@material-ui/core/styles';


const styles = (theme) => ({
  appBar: {
    zIndex: theme.zIndex.drawer + 1,
  },
});

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
    const { classes } = this.props;
    return (
      <AppBar position='absolute' className={classes.appBar}>
        <MaterialToolbar>
          <Typography variant='title' color='inherit'>
            dSpaceX
          </Typography>
          <div style={{
            width: '100%',
            display: 'flex',
            flexDirection: 'row-reverse',
          }}>
            <Button variant='raised' style={{ backgroundColor:'white', marginLeft:'5px' }}
              disabled={!this.props.dataset}>
              <Icon path={mdiFilter} size={0.75} color='grey' style={{ paddingRight:'5px' }}/>
              Filter
            </Button>
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

Toolbar.propTypes = {
  classes: PropTypes.object.isRequired,
};

export default withStyles(styles)(Toolbar);
